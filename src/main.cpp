#include "X402Handler.h"

#include <proxygen/httpserver/HTTPServer.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <wangle/acceptor/Acceptor.h>
#include <wangle/ssl/SSLContextConfig.h>
#include <folly/init/Init.h>

using namespace proxygen;

class X402HandlerFactory : public RequestHandlerFactory {
public:
    void onServerStart(folly::EventBase*) noexcept override {}
    void onServerStop() noexcept override {}

    RequestHandler* onRequest(RequestHandler*, HTTPMessage* msg) noexcept override {
        // Route if needed (e.g., only gate /paid). Here we gate everything.
        (void)msg;
        return new X402Handler();
    }
};

int main(int argc, char* argv[]) {
    folly::Init init(&argc, &argv);

    HTTPServer::IPConfig ipConfig(
        folly::SocketAddress("0.0.0.0", 8080, true), HTTPServer::Protocol::HTTP);

    HTTPServerOptions options;
    options.threads = static_cast<size_t>(std::thread::hardware_concurrency());
    options.idleTimeout = std::chrono::milliseconds(60000);
    options.shutdownOn = {SIGINT, SIGTERM};
    options.handlerFactories = RequestHandlerChain()
        .addThen<X402HandlerFactory>()
        .build();

    HTTPServer server(std::move(options));
    server.bind({ipConfig});

    server.start();
    server.join();
    return 0;
}
