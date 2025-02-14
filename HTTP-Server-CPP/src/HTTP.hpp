#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

enum class HTTP_Request_Type : uint8_t {
	HTTP_GET,
	HTTP_POST,
	HTTP_PUT,
	HTTP_DELETE,
	HTTP_INVALID_REQUST
};

enum class HTTP_Response_Code : uint16_t {
	HTTP_OK = 200,
	HTTP_BAD_REQUEST = 400,
	HTTP_NOT_FOUND = 404,
};

struct HTTP_Request {
	std::string HTTPversion;
	HTTP_Request_Type type;
	std::string path;
	std::unordered_map<std::string, std::vector<std::string>> headers;

};

struct HTTP_Response {
	HTTP_Response_Code code;
	std::string contentType;
	std::string contentLength;
	std::string content;
	std::string time;

};