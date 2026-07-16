#pragma once

#include <atomic>
#include <chrono>
#include <optional>

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

private:
	// GPS_RAW_INT fix_type: 0-1 no fix, 2 = 2D, 3 = 3D, ...
	static constexpr uint8_t kMinFixType = 2;
	static constexpr std::chrono::seconds kStableFixDuration{3};

	bool wait_for_mavsdk_connection(double timeout_s);

	void send_mavlink_gps_rtcm_data(const mavlink_gps_rtcm_data_t& msg);
	void handle_gps_raw_int(const mavlink_message_t& message);

	struct ECEFPosition {
		double x;
		double y;
		double z;
		bool updated;
		std::mutex lock;
	} _ecef_position = {};

	// Tracks continuous >=2D fix so we only send after a stable window.
	std::optional<std::chrono::steady_clock::time_point> _fix_ok_since;

	// MAVSDK
	std::shared_ptr<mavsdk::Mavsdk> _mavsdk;
	std::shared_ptr<mavsdk::MavlinkPassthrough> _mavlink_passthrough;
	uint8_t _sequence_id = 0;
	// Polaris
	std::shared_ptr<point_one::polaris::PolarisClient> _polaris_client;
	std::atomic<bool> _gps_position_set = false;
	// Other
	Settings _settings;
	std::atomic<bool> _should_exit = false;
	std::atomic<bool> _exiting = false;
};
