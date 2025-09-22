
#include "ServerFactory.h"
#include "X402HandlerFactory.h"


#include <proxygen/httpserver/HTTPServer.h>
#include <proxygen/httpserver/HTTPServerOptions.h>

#include <glog/logging.h>

using namespace proxygen;

std::shared_ptr<HTTPServer> ServerFactory::createServerInstance(std::string bindIP, uint64_t bindPort) {



    HTTPServer::IPConfig ipConfig(
        folly::SocketAddress(bindIP, bindPort, true), HTTPServer::Protocol::HTTP);

    HTTPServerOptions options;
    LOG(INFO) << "Creating server instance ...";
    options.threads = static_cast<size_t>(std::thread::hardware_concurrency());
    options.idleTimeout = std::chrono::milliseconds(60000);
    options.shutdownOn = {SIGINT, SIGTERM};
    options.handlerFactories = RequestHandlerChain()
            .addThen<X402HandlerFactory>()
            .build();

    auto server = std::make_shared<HTTPServer>(std::move(options));
    LOG(INFO) << "Binding server to address...";
    server->bind({ipConfig});
    LOG(INFO) << "Server instance created and bound successfully.";
    return server;
}
