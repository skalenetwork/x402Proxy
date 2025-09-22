#pragma once

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <folly/json.h>
#include <string>

class X402Handler : public proxygen::RequestHandler {
public:
    void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;
    void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
    void onEOM() noexcept override;
    void requestComplete() noexcept override { delete this; }
    void onError(proxygen::ProxygenError) noexcept override { delete this; }

private:
    bool hasValidPaymentHeader(const proxygen::HTTPMessage* req, std::string& paymentInfo);
    void reply402();

    std::unique_ptr<proxygen::HTTPMessage> reqHeaders_;
    std::string bodyBuffer_;
};
