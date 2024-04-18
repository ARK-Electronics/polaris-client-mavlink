#include "PolarisClientMavlink.hpp"
#include <iomanip>
#include <iostream>
#include <future>
#include <regex>
#include <algorithm>
#include <mavsdk/log_callback.h>

PolarisClientMavlink::PolarisClientMavlink(const PolarisClientMavlink::Settings& settings)
	: _settings(settings)
{
	// Disable mavsdk noise
	mavsdk::log::subscribe([](...) {
		// https://mavsdk.mavlink.io/main/en/cpp/guide/logging.html
		return true;
	});
}

void PolarisClientMavlink::stop()
{
	_polaris_client->Disconnect();
	_should_exit = true;
}

bool PolarisClientMavlink::wait_for_mavsdk_connection(double timeout_ms)
{
	std::cout << "Connecting to " << _settings.mavsdk_connection_url << std::endl;
	_mavsdk = std::make_shared<mavsdk::Mavsdk>(mavsdk::Mavsdk::Configuration(1, MAV_COMP_ID_ONBOARD_COMPUTER,
			true)); // Emit heartbeats (Client)
	auto result = _mavsdk->add_any_connection(_settings.mavsdk_connection_url);

	if (result != mavsdk::ConnectionResult::Success) {
		std::cout << "Connection failed: " << result << std::endl;
		return false;
	}

	auto system = _mavsdk->first_autopilot(timeout_ms);

	if (!system) {
		std::cout << "Timed out waiting for system" << std::endl;
		return false;
	}

	std::cout << "Connected to autopilot" << std::endl;

	// MAVSDK plugins
	_mavlink_passthrough = std::make_shared<mavsdk::MavlinkPassthrough>(system.value());

	return true;
}

void PolarisClientMavlink::RTCMCallback(const uint8_t* recv, size_t length)
{
	std::cout << "\nReceived: " << length << std::endl;

	mavlink_gps_rtcm_data_t msg = {};

	if (length <= MAVLINK_MSG_GPS_RTCM_DATA_FIELD_DATA_LEN) {
		msg.len = length;
		msg.flags = (_sequence_id & 0x1F) << 3;
		memcpy(msg.data, recv, length);
		// Send mavlink message
		send_mavlink_gps_rtcm_data(msg);

	} else {

		uint8_t fragment_id = 0;         // Fragment id indicates the fragment within a set
		int start = 0;

		while (start < length) {
			int current_length = std::min(int(length) - start, MAVLINK_MSG_GPS_RTCM_DATA_FIELD_DATA_LEN);
			msg.flags = 1; // LSB set indicates message is fragmented
			msg.flags |= (fragment_id++ & 0x3) << 1; // Next 2 bits are fragment id
			msg.flags |= (_sequence_id & 0x1F) << 3; // Next 5 bits are sequence id
			msg.len = current_length;
			memcpy(&msg.data, recv + start, current_length);
			send_mavlink_gps_rtcm_data(msg);
			start += current_length;
		}
	}

	_sequence_id++;

	if (_sequence_id >= 32) {
		_sequence_id = 0;
	}
}

void PolarisClientMavlink::send_mavlink_gps_rtcm_data(const mavlink_gps_rtcm_data_t& msg)
{
	std::cout << "send_mavlink_gps_rtcm_data: " << int(msg.len) << std::endl;
	_mavlink_passthrough->queue_message([&](MavlinkAddress mavlink_address, uint8_t channel) {
		mavlink_message_t message;

		mavlink_msg_gps_rtcm_data_encode_chan(
			mavlink_address.system_id,
			mavlink_address.component_id,
			channel,
			&message,
			&msg);
		return message;
	});
}

void PolarisClientMavlink::handle_gps_raw_int(const mavlink_message_t& message)
{
	if (_gps_position_set) {
		// TODO: unsubscribe?
		return;
	}

	mavlink_gps_raw_int_t msg;
	mavlink_msg_gps_raw_int_decode(&message, &msg);

	// Convert WSG to ECEF
	double A_EARTH = 6378137.0;
	double flattening = 1.0 / 298.257223563;
	double NAV_E2 = (2.0 - flattening) * flattening; // also e^2
	double deg2rad = M_PI / 180.0;

	double lat = double(msg.lat) / 1e7;
	double lon = double(msg.lon) / 1e7;
	double alt = double(msg.alt) / 1e7;

	double slat = sin(lat * deg2rad);
	double clat = cos(lat * deg2rad);
	double r_n = A_EARTH / sqrt(1.0 - NAV_E2 * slat * slat);
	double x = (r_n + alt) * clat * cos(lon * deg2rad);
	double y = (r_n + alt) * clat * sin(lon * deg2rad);
	double z = (r_n * (1.0 - NAV_E2) + alt) * slat;

	std::cout << "Updating GPS position" << std::endl;

	std::lock_guard<std::mutex> lock(_ecef_position.lock);
	_ecef_position.x = x;
	_ecef_position.y = y;
	_ecef_position.z = z;
	_ecef_position.updated = true;
}

void PolarisClientMavlink::run()
{
	std::srand(std::time(0));
	std::string session_id = std::to_string(std::rand());
	std::cout << "Session ID: " << session_id << std::endl;
	// Create polaris client
	_polaris_client = std::make_shared<point_one::polaris::PolarisClient>(_settings.polaris_api_key, session_id);
	_polaris_client->SetRTCMCallback(std::bind(&PolarisClientMavlink::RTCMCallback, this, std::placeholders::_1, std::placeholders::_2));
	_polaris_client->RunAsync();

	// Set up callbacks for GPS message so we can set position: SendECEFPosition
	auto gps_callback = [this](const mavlink_message_t& message) { handle_gps_raw_int(message); };
	_mavlink_passthrough->subscribe_message(
		MAVLINK_MSG_ID_GPS_RAW_INT,
		gps_callback
	);

	while (!_should_exit) {
		if (_ecef_position.updated && !_gps_position_set) {
			_ecef_position.lock.lock();
			_ecef_position.updated = false;
			double x = _ecef_position.x;
			double y = _ecef_position.y;
			double z = _ecef_position.z;
			_ecef_position.lock.unlock();
			std::cout << "Sending ECEF Position to Polaris" << std::endl;
			_polaris_client->SendECEFPosition(x, y, z);
			_gps_position_set = true;
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}


