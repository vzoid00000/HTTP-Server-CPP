#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

struct Server_Settings {
	std::string port;
	std::string documentsPath;
};

class Server {
public:
	Server();
	void run();

	[[nodiscard]] inline const std::string GetPort() const {
		return m_Settings.port;
	}

	[[nodiscard]] inline const std::string GetDocumentsPath() const {
		return m_Settings.documentsPath;
	}

	[[nodiscard]] inline const Server_Settings GetServerSettings() const {
		return m_Settings;
	}

private:
	bool readSettings();

private:
	Server_Settings m_Settings{};
};