#pragma once


#include <string>
#include <cstdint>
#include <memory>
namespace proxygen {
    class HTTPServer;
}

class ServerFactory {
public:
    std::shared_ptr<proxygen::HTTPServer> createAndStartServerInstance(std::string bindIP, uint64_t bindPort);
};

