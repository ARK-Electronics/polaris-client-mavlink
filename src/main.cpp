#include "PolarisClientMavlink.hpp"
#include <filesystem>
#include <signal.h>
#include <iostream>
#include <toml.hpp>
#include <unistd.h>
#include <sys/types.h>

static void signal_handler(int signum);

std::shared_ptr<PolarisClientMavlink> _polaris_client_mavlink;

int main(int argc, char** argv)
{
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	setbuf(stdout, NULL); // Disable stdout buffering

	// Config lookup: --config <path> (or --config=<path>) overrides everything;
	// otherwise user override > deb-installed default.
	const std::string home = getenv("HOME") ? getenv("HOME") : "/tmp";
	const auto user_config = std::filesystem::path(home) / ".config/ark/polaris/config.toml";
	const auto default_config = std::filesystem::path("/opt/ark/share/polaris/config.toml");
	std::string config_path = (std::filesystem::exists(user_config) ? user_config : default_config).string();

	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];

		if (arg == "--config" && i + 1 < argc) {
			config_path = argv[++i];

		} else if (arg.rfind("--config=", 0) == 0) {
			config_path = arg.substr(std::string("--config=").size());
		}
	}

	toml::table config;

	try {
		config = toml::parse_file(config_path);

	} catch (const toml::parse_error& err) {
		std::cerr << "Parsing failed:\n" << err << "\n";
		return -1;

	} catch (const std::exception& err) {
		std::cerr << "Error: " << err.what() << "\n";
		return -1;
	}

	PolarisClientMavlink::Settings settings = {
		.mavsdk_connection_url = config["connection_url"].value_or("0.0.0"),
		.polaris_api_key = config["polaris_api_key"].value_or("<your_key_goes_here>")
	};

	_polaris_client_mavlink = std::make_shared<PolarisClientMavlink>(settings);

	_polaris_client_mavlink->run();

	std::cout << "exiting" << std::endl;

	return 0;
}

static void signal_handler(int signum)
{
	if (_polaris_client_mavlink.get()) _polaris_client_mavlink->stop();
}
