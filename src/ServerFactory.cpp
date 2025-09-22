
#include "ServerFactory.h"
#include "X402HandlerFactory.h"


#include <proxygen/httpserver/HTTPServer.h>
#include <proxygen/httpserver/HTTPServerOptions.h>

using namespace proxygen;

std::shared_ptr<HTTPServer> ServerFactory::createAndStartServerInstance(std::string bindIP, uint64_t bindPort) {
    HTTPServer::IPConfig ipConfig(
        folly::SocketAddress(bindIP, bindPort, true), HTTPServer::Protocol::HTTP);

    HTTPServerOptions options;
    options.threads = static_cast<size_t>(std::thread::hardware_concurrency());
    options.idleTimeout = std::chrono::milliseconds(60000);
    options.shutdownOn = {SIGINT, SIGTERM};
    options.handlerFactories = RequestHandlerChain()
            .addThen<X402HandlerFactory>()
            .build();

    auto server = std::make_shared<HTTPServer>(std::move(options));
    server->bind({ipConfig});
    return server;
}
