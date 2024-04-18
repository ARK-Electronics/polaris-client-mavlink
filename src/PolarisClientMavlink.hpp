#pragma once

#include <atomic>

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>

#include <point_one/polaris/polaris_client.h>

class PolarisClientMavlink
{
public:
	struct Settings {
		std::string mavsdk_connection_url;
		std::string polaris_api_key;
	};

	PolarisClientMavlink(const Settings& settings);

	void RTCMCallback(const uint8_t* recv, size_t length);

	void run();
	void stop();
	bool wait_for_mavsdk_connection(double timeout_ms);

private:
	void send_mavlink_gps_rtcm_data(const mavlink_gps_rtcm_data_t& msg);
	void handle_gps_raw_int(const mavlink_message_t& message);

	struct ECEFPosition {
		double x;
		double y;
		double z;
		bool updated;
		std::mutex lock;
	} _ecef_position;

	// MAVSDK
	std::shared_ptr<mavsdk::Mavsdk> _mavsdk;
	std::shared_ptr<mavsdk::MavlinkPassthrough> _mavlink_passthrough;
	// Polaris
	std::shared_ptr<point_one::polaris::PolarisClient> _polaris_client;
	// Other
	Settings _settings;
	uint8_t _sequence_id = 0;
	std::atomic<bool> _gps_position_set = false;
	std::atomic<bool> _should_exit = false;
	std::atomic<bool> _exiting = false;
};
