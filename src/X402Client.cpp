#include "X402Client.h"

X402Client::X402Client(const std::string &connect_ip, uint16_t port)
    : connectIp(connect_ip), port(port) {}

X402Client::~X402Client() {}

std::string X402Client::baseUrl() {
    return "http://" + connectIp;
}

std::string X402Client::parseStatusLineAndHeaders(const std::vector<std::string> &headersVector,
                                                  std::map<std::string, std::string> &headersMap) {
    std::string statusLine;
    if (!headersVector.empty()) {
        statusLine = headersVector[0];
        statusLine.erase(statusLine.find_last_not_of(" \t\r\n") + 1);
    }
    for (size_t i = 1; i < headersVector.size(); ++i) {
        auto pos = headersVector[i].find(':');
        if (pos != std::string::npos) {
            std::string key = headersVector[i].substr(0, pos);
            std::string value = headersVector[i].substr(pos + 1);
            key.erase(0, key.find_first_not_of(" \t\r\n"));
            key.erase(key.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            headersMap[key] = value;
        }
    }
    return statusLine;
}

std::tuple<std::map<std::string, std::string>, std::string, HttpResponse>
X402Client::sendRequestAndParseResult(std::string location, const std::vector<std::string> &extraHeaders) {
    auto resp = httpGet(baseUrl(), location, extraHeaders);
    auto headersVector = resp.headers;
    std::map<std::string, std::string> headersMap;
    auto statusLine = parseStatusLineAndHeaders(headersVector, headersMap);
    LOG(INFO) << "STATUS::" << statusLine;
    for (const auto &[key, value]: headersMap) {
        LOG(INFO) << key << ": " << value;
    }
    LOG(INFO) << "BODY::" << resp.body;
    return {headersMap, statusLine, resp};
}

size_t X402Client::writeBody(char *ptr, size_t size, size_t nmemb, void *userdata) {
    auto *out = static_cast<std::string *>(userdata);
    out->append(ptr, size * nmemb);
    return size * nmemb;
}

size_t X402Client::writeHeader(char *buffer, size_t size, size_t nitems, void *userdata) {
    auto *headers = static_cast<std::vector<std::string> *>(userdata);
    headers->emplace_back(buffer, size * nitems);
    return size * nitems;
}

HttpResponse X402Client::httpGet(const std::string &_baseURL, const std::string &_location,
                                 const std::vector<std::string> &extraHeaders) {
    CURL *curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl_easy_init failed");

    std::string url = _baseURL + ":" + std::to_string(port) + "/" + _location;

    struct curl_slist *hdrs = nullptr;
    for (auto &h: extraHeaders) hdrs = curl_slist_append(hdrs, h.c_str());

    HttpResponse resp;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeBody);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp.body);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writeHeader);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &resp.headers);
    if (hdrs)
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);

    auto rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        if (hdrs) curl_slist_free_all(hdrs);
        curl_easy_cleanup(curl);
        throw std::runtime_error(std::string("curl perform error: ") + curl_easy_strerror(rc));
    }
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp.status);

    if (hdrs) curl_slist_free_all(hdrs);
    curl_easy_cleanup(curl);
    return resp;
}