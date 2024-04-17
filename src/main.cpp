#include "PolarisRTKClient.hpp"
#include <filesystem>
#include <signal.h>
#include <iostream>
#include <toml.hpp>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

static void signal_handler(int signum);

static std::string get_user_name()
{
	uid_t uid = geteuid();
	struct passwd* pw = getpwuid(uid);

	if (pw) {
		return std::string(pw->pw_name);
	}

	return {};
}

std::atomic<bool> _should_exit = false;
std::shared_ptr<PolarisRTKClient> _polaris_rtk_client;

int main(int argc, char* argv[])
{
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	setbuf(stdout, NULL); // Disable stdout buffering

	toml::table config;

	std::string default_config_path = "config.toml";
	bool config_exists = std::filesystem::exists(default_config_path);

	try {
		std::string config_path = config_exists ? default_config_path : "/home/" + get_user_name() + "/polaris-rtk-client/config.toml";
		config = toml::parse_file(config_path);

	} catch (const toml::parse_error& err) {
		std::cerr << "Parsing failed:\n" << err << "\n";
		return -1;

	} catch (const std::exception& err) {
		std::cerr << "Error: " << err.what() << "\n";
		return -1;
	}

	// Setup the LogLoader
	PolarisRTKClient::Settings settings = {
		.mavsdk_connection_url = config["connection_url"].value_or("0.0.0"),
		.polaris_api_key = config["polaris_api_key"].value_or("<your_key_goes_here>")
	};

	_polaris_rtk_client = std::make_shared<PolarisRTKClient>(settings);

	bool connected = false;

	while (!_should_exit && !connected) {
		connected = _polaris_rtk_client->wait_for_mavsdk_connection(3);
	}

	if (!_should_exit && connected) {
		_polaris_rtk_client->run();
	}

	std::cout << "exiting" << std::endl;

	return -1;
}

static void signal_handler(int signum)
{
	if (_polaris_rtk_client.get()) _polaris_rtk_client->stop();

	_should_exit = true;
}
