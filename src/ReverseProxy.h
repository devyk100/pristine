#ifndef REVERSE_PROXY_H
#define REVERSE_PROXY_H

#include "ConfigManager.h"
#include "RequestRouter.h"
#include "ConnectionHandler.h"
#include "CertificateManager.h"
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <memory>
#include <thread>
#include <vector>

namespace beast = boost::beast;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

class ReverseProxy {
public:
    ReverseProxy();
    ~ReverseProxy();
    
    bool initialize(const std::string& configPath);
    void run();
    void stop();

private:
    void start_http_server();
    void start_https_server();
    void accept_http_connections();
    void accept_https_connections();
    void on_http_accept(beast::error_code ec, tcp::socket socket);
    void on_https_accept(beast::error_code ec, tcp::socket socket);
    
    // SSL context setup
    void setup_ssl_context();

private:
    net::io_context ioc_;
    std::vector<std::thread> threads_;
    
    // HTTP server
    std::unique_ptr<tcp::acceptor> http_acceptor_;
    
    // HTTPS server
    std::unique_ptr<tcp::acceptor> https_acceptor_;
    std::unique_ptr<ssl::context> ssl_ctx_;
    
    // Core components
    std::shared_ptr<ConfigManager> config_manager_;
    std::shared_ptr<RequestRouter> router_;
    std::shared_ptr<CertificateManager> cert_manager_;
    
    bool running_;
};

#endif // REVERSE_PROXY_H
