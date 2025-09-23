#pragma once

#include "../src/common.h"
#include <folly/init/Init.h>
#include <folly/SocketAddress.h>
#include <folly/init/Init.h>
#include <proxygen/httpserver/HTTPServer.h>
#include "ServerFactory.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

struct HttpResponse {
    long status = 0;
    std::string body;
    std::vector<std::string> headers;
};

struct X402Client {
private:
    std::string connectIp;
    uint16_t port;

public:
    X402Client(const std::string &_connectIp, uint16_t _port);
    ~X402Client();

    std::string baseUrl();
    static std::string parseStatusLineAndHeaders(const std::vector<std::string> &_headersVector,
                                                 std::map<std::string, std::string> &_headersMap);
    std::tuple<std::map<std::string, std::string>, std::string, HttpResponse>
    sendRequestAndParseResult(std::string _location, const std::vector<std::string> &_extraHeaders);

    static size_t writeBody(char *_ptr, size_t _size, size_t _nmemb, void *_userdata);
    static size_t writeHeader(char *_buffer, size_t _size, size_t _nitems, void *_userdata);

    HttpResponse httpGet(const std::string &_baseURL, const std::string &_location,
                         const std::vector<std::string> &_extraHeaders);
};




