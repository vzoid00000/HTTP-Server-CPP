#include "Config.hpp"
#include "HTTP.hpp"
#include "Server.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <filesystem>
#include <chrono>
#include <format>

const std::string getCurrentTime() {

	auto now = std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::system_clock::now() };
	auto truncated_now = std::chrono::zoned_time{ now.get_time_zone(), std::chrono::floor<std::chrono::seconds>(now.get_sys_time()) };

	return std::format("{:%a, %d %b %Y %T %Z}", truncated_now);
}

const std::string parseHTTPResponse(const HTTP_Response& httpResponse) {
	std::string respone = "HTTP/1.1 " + std::to_string(static_cast<uint16_t>(httpResponse.code));

	switch (httpResponse.code) {
	case HTTP_Response_Code::HTTP_OK: {
		respone += " OK\n";
		break;
	}
	case HTTP_Response_Code::HTTP_NOT_FOUND: {
		respone += " Not Found\n";
		break;
	}
	default:
		break;
	}

	respone += "Content-Type: " + httpResponse.contentType + "\nContent-Length: " + httpResponse.contentLength + "\nDate: " + httpResponse.time + "\n\n";
	respone += httpResponse.content;

	return respone;
}

void handleResponse(const SOCKET connectionSocket, const HTTP_Request& request, const std::string& documentsPath) {

	HTTP_Response response{};

	switch (request.type) {

	case HTTP_Request_Type::HTTP_GET: {

		response.time = getCurrentTime();
		std::string localPath = documentsPath + request.path;

		if (!std::filesystem::exists(documentsPath) || !std::filesystem::is_directory(documentsPath)) {
			response.code = HTTP_Response_Code::HTTP_NOT_FOUND;
			response.contentType = "application/json";
			response.content = R"({
  "error": "Not Found",
  "message": "Server file handling error"
})";
			response.contentLength = std::to_string(response.content.size());
		}
		else if (localPath == documentsPath + '/') {
			localPath += "index.html";

			if (!std::filesystem::exists(localPath) || !std::filesystem::is_regular_file(localPath)) {
				response.code = HTTP_Response_Code::HTTP_NOT_FOUND;
				response.contentType = "application/json";
				response.content = R"({
  "error": "Not Found",
  "message": "Could not find file"
})";
				response.contentLength = std::to_string(response.content.size());
			}
			else {
				std::ifstream file(localPath, std::ios::binary);

				if (file.is_open()) {
					response.content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
					file.close();
				}
				response.code = HTTP_Response_Code::HTTP_OK;
				response.contentType = "text/html";
				response.contentLength = std::to_string(response.content.size());

			}
		}
		else {
			if (!std::filesystem::exists(localPath) || !std::filesystem::is_regular_file(localPath)) {
				response.code = HTTP_Response_Code::HTTP_NOT_FOUND;
				response.contentType = "application/json";
				response.content = R"({
  "error": "Not Found",
  "message": "Could not find file"
})";
				response.contentLength = std::to_string(response.content.size());
			}
			else {
				std::ifstream file(localPath, std::ios::binary);

				if (file.is_open()) {
					response.content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
					file.close();
				}
				response.code = HTTP_Response_Code::HTTP_OK;
				response.contentLength = std::to_string(response.content.size());

				size_t fileExtensionDotPos = localPath.find('.');
				if (fileExtensionDotPos == std::string::npos) {
					response.contentType = "text/plain";
				}
				else if (localPath.substr(fileExtensionDotPos + 1) == "html") {
					response.contentType = "text/html";
				}
				else if (localPath.substr(fileExtensionDotPos + 1) == "css") {
					response.contentType = "text/css";
				}
				else if (localPath.substr(fileExtensionDotPos + 1) == "js") {
					response.contentType = "text/javascript";
				}
				else if (localPath.substr(fileExtensionDotPos + 1) == "jpg" || localPath.substr(fileExtensionDotPos + 1) == "jpeg") {
					response.contentType = "image/jpeg";
				}
				else if (localPath.substr(fileExtensionDotPos + 1) == "ico") {
					response.contentType = "image/x-icon";
				}
				else if (localPath.substr(fileExtensionDotPos + 1) == "json") {
					response.contentType = "application/json";
				}


			}
		}

		break;
	}

	case HTTP_Request_Type::HTTP_POST: {
		break;
	}

	case HTTP_Request_Type::HTTP_PUT: {
		break;
	}

	case HTTP_Request_Type::HTTP_DELETE: {
		break;
	}

	case HTTP_Request_Type::HTTP_INVALID_REQUST: {
		response.code = HTTP_Response_Code::HTTP_BAD_REQUEST;
		response.contentType = "application/json";
		response.content = R"({"error": "Bad request", "message": "Invalid HTTP request format",})";
		response.contentLength = std::to_string(response.content.size());
		break;
	}

	default: {
		break;
	}

	}


	const std::string responseString = parseHTTPResponse(response);
	std::cout << responseString << std::endl;
	int result = 0;

	result = send(connectionSocket, responseString.c_str(), static_cast<int>(responseString.size()), 0);
	if (result == SOCKET_ERROR) {
		std::cout << "send failed: " << WSAGetLastError() << std::endl;
		closesocket(connectionSocket);
		WSACleanup();
	}

}

const HTTP_Request parseHTTPRequest(const std::string& httpRequest) {
	std::istringstream httpRequestStream(httpRequest);
	std::string currentLine;
	size_t count = 0;
	HTTP_Request request{};

	while (std::getline(httpRequestStream, currentLine)) {
		std::istringstream currentLineStream(currentLine);

		std::string currentWord;
		std::vector<std::string> words;
		while (currentLineStream >> currentWord) {
			words.push_back(currentWord);
		}

		// start-line
		if (count == 0) {
			if (words.size() != 3) {
				request.type = HTTP_Request_Type::HTTP_INVALID_REQUST;
				break;
			}
			else if (words[0] != "GET" && words[0] != "POST" && words[0] != "PUT" && words[0] != "DELETE") {
				request.type = HTTP_Request_Type::HTTP_INVALID_REQUST;
				break;
			}
			else if (words[1][0] != '/') {
				request.type = HTTP_Request_Type::HTTP_INVALID_REQUST;
				break;
			}

			if (words[0] == "GET") {
				request.type = HTTP_Request_Type::HTTP_GET;
			}
			else if (words[0] == "POST") {
				request.type = HTTP_Request_Type::HTTP_POST;
			}
			else if (words[0] == "PUT") {
				request.type = HTTP_Request_Type::HTTP_PUT;
			}
			else if (words[0] == "DELETE") {
				request.type = HTTP_Request_Type::HTTP_DELETE;
			}

			request.path = words[1];

		}
		else if (words.size() > 1) {
			bool isFirst = true;
			for (const auto& currentWord : words) {
				if (isFirst) {
					isFirst = false;
					continue;
				}
				request.headers[words[0]].push_back(currentWord);
			}
		}
		count++;
	}

	return request;
}

const HTTP_Request handleRequest(const SOCKET connectionSocket) {
	int result = 0;
	std::string requestString;
	char recvbuf[Config::RECV_BUFFER_SIZE];
	do {
		result = recv(connectionSocket, recvbuf, Config::RECV_BUFFER_SIZE - 1, 0);
		if (result > 0) {
			//std::cout << "bytes received: " << result << '\n';
			recvbuf[result] = '\0';
			requestString += recvbuf;

			if (requestString.find("\r\n\r\n") != std::string::npos) break;

		}
		else if (result == 0) {
			std::cout << "connection closed by client\n";
		}
		else {
			std::cout << "connection abruptly closed by client\n";
		}

	} while (result > 0);

	return parseHTTPRequest(requestString);
}

void shutdownConnection(const SOCKET connectionSocket) {
	int result = 0;

	result = shutdown(connectionSocket, SD_SEND);
	if (result == SOCKET_ERROR) {
		std::cout << "shutdown failed, code: " << WSAGetLastError() << std::endl;
		closesocket(connectionSocket);
		WSACleanup();
		std::cin.get();
		return;
	}
	closesocket(connectionSocket);
}

void handleClient(const SOCKET connectionSocket, const std::string& documentsPath) {

	const HTTP_Request request = handleRequest(connectionSocket);
	handleResponse(connectionSocket, request, documentsPath);

	shutdownConnection(connectionSocket);
}

Server::Server() {}

void Server::run() {

	if (!readSettings()) {
		std::cout << "Reading settings.txt failed" << std::endl;
		return;
	}

	int result = 0;

	// init winsock
	WSADATA wsaData;
	result = WSAStartup(MAKEWORD(Config::WINSOCK_MAJOR_VERSION, Config::WINSOCK_MINOR_VERSION), &wsaData);
	if (result) {
		std::cout << "WSAStartup failed, code: " << result << std::endl;
		std::cin.get();
		return;
	}


	addrinfo addressInfo;

	ZeroMemory(&addressInfo, sizeof(addressInfo));
	addressInfo.ai_family = AF_INET;
	addressInfo.ai_socktype = SOCK_STREAM;
	addressInfo.ai_protocol = IPPROTO_TCP;
	addressInfo.ai_flags = AI_PASSIVE;

	PADDRINFOA addressInfoResult = nullptr;
	result = getaddrinfo(nullptr, GetPort().c_str(), &addressInfo, &addressInfoResult);
	if (result) {
		std::cout << "getaddrinfo failed, code: " << result << std::endl;
		WSACleanup();
		std::cin.get();
		return;
	}


	// create socket
	SOCKET listenSocket = INVALID_SOCKET;
	listenSocket = socket(addressInfoResult->ai_family, addressInfoResult->ai_socktype, addressInfoResult->ai_protocol);
	if (listenSocket == INVALID_SOCKET) {
		std::cout << "socket failed, code:" << WSAGetLastError() << std::endl;
		freeaddrinfo(addressInfoResult);
		WSACleanup();
		std::cin.get();
		return;
	}

	// bind socket
	result = bind(listenSocket, addressInfoResult->ai_addr, (int)addressInfoResult->ai_addrlen);
	if (result == SOCKET_ERROR) {
		std::cout << "bind failed, code: " << WSAGetLastError() << std::endl;
		freeaddrinfo(addressInfoResult);
		closesocket(listenSocket);
		WSACleanup();
		std::cin.get();
		return;
	}
	freeaddrinfo(addressInfoResult);

	// start listening
	result = listen(listenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR) {
		std::cout << "listen failed, code: " << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		std::cin.get();
		return;
	}

	while (true) {
		// accept incoming client
		SOCKET connectionSocket = INVALID_SOCKET;

		connectionSocket = accept(listenSocket, nullptr, nullptr);
		if (connectionSocket == SOCKET_ERROR) {
			std::cout << "accept failed, code: " << WSAGetLastError() << std::endl;
			closesocket(listenSocket);
			WSACleanup();
			std::cin.get();
			return;
		}

		std::thread clientHandler(&handleClient, connectionSocket, GetDocumentsPath());
		clientHandler.detach();
	}


	result = shutdown(listenSocket, SD_SEND);
	if (result == SOCKET_ERROR) {
		std::cout << "shutdown failed, code: " << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		std::cin.get();
		return;
	}
	closesocket(listenSocket);

	WSACleanup();
}


bool Server::readSettings() {

	if (!std::filesystem::exists(Config::SETTINGS_FILE_NAME) || !std::filesystem::is_regular_file(Config::SETTINGS_FILE_NAME)) {
		std::ofstream settingsFile(Config::SETTINGS_FILE_NAME);

		if (settingsFile.is_open()) {

			settingsFile << R"(port: 80
documentsPath: files)";
			settingsFile.close();
		}


		m_Settings.port = "80";
		m_Settings.documentsPath = "files";
	}
	else {
		std::ifstream settingsFile(Config::SETTINGS_FILE_NAME, std::ios::binary);
		std::string settingsFileContent;

		if (settingsFile.is_open()) {
			settingsFileContent = std::string((std::istreambuf_iterator<char>(settingsFile)), std::istreambuf_iterator<char>());
			settingsFile.close();
		}

		std::istringstream settingsFileContentStream(settingsFileContent);
		std::string currentLine;

		while (std::getline(settingsFileContentStream, currentLine)) {
			std::istringstream currentLineStream(currentLine);

			std::string currentWord;
			std::vector<std::string> words;
			while (currentLineStream >> currentWord) {
				words.push_back(currentWord);
			}

			if (words.size() != 2) {
				std::cout << "Setting file format is wrong/corrupted\n" << std::endl;
				return false;
			}

			if (words[0] == "port:") {
				m_Settings.port = words[1];
			}
			else if (words[0] == "documentsPath:") {
				m_Settings.documentsPath = words[1];
			}
			else {
				std::cout << "Unkown settings configuration\n" << std::endl;
				return false;
			}
		}
	}

	std::cout << "port: " << m_Settings.port << '\n';
	std::cout << "documentsPath: " << m_Settings.documentsPath << '\n';
	return true;
}


void listenSocket() {

}
