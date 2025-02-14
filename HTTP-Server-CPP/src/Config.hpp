#pragma once
#include <cstdint>
#include <string>

namespace Config {
	constexpr uint8_t WINSOCK_MAJOR_VERSION = 2;
	constexpr uint8_t WINSOCK_MINOR_VERSION = 2;
	constexpr uint32_t RECV_BUFFER_SIZE = 1024;
	const std::string SETTINGS_FILE_NAME = "settings.txt";
}