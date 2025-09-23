#pragma once

#include "../src/common.h"


#include <folly/init/Init.h>
#include <folly/SocketAddress.h>
#include <folly/init/Init.h>
#include <proxygen/httpserver/HTTPServer.h>

#include "ServerFactory.h"

// ---- libcurl helper ---------------------------------------------------------
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
    X402Client(const std::string &connect_ip, uint16_t port);
    ~X402Client();

    std::string baseUrl();
    static std::string parseStatusLineAndHeaders(const std::vector<std::string> &headersVector,
                                                 std::map<std::string, std::string> &headersMap);
    std::tuple<std::map<std::string, std::string>, std::string, HttpResponse>
    sendRequestAndParseResult(std::string location, const std::vector<std::string> &extraHeaders);

    static size_t writeBody(char *ptr, size_t size, size_t nmemb, void *userdata);
    static size_t writeHeader(char *buffer, size_t size, size_t nitems, void *userdata);

    HttpResponse httpGet(const std::string &_baseURL, const std::string &_location,
                         const std::vector<std::string> &extraHeaders);
};




