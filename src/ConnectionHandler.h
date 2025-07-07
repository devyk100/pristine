#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

#include "RequestRouter.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

class ConnectionHandler : public std::enable_shared_from_this<ConnectionHandler> {
public:
    ConnectionHandler(
        tcp::socket&& socket,
        std::shared_ptr<RequestRouter> router,
        bool is_ssl = false
    );
    
    ConnectionHandler(
        beast::ssl_stream<beast::tcp_stream>&& ssl_socket,
        std::shared_ptr<RequestRouter> router
    );
    
    void start();

private:
    void do_read();
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void handle_request();
    void forward_to_backend();
    void handle_websocket_upgrade();
    void on_backend_connect(beast::error_code ec);
    void on_backend_write(beast::error_code ec, std::size_t bytes_transferred);
    void on_backend_read(beast::error_code ec, std::size_t bytes_transferred);
    void send_error_response(http::status status, const std::string& message);
    void close_connection();
    
    // Extract host from request
    std::string extract_host_from_request();
    
    // Copy headers from source to target
    template<bool isRequest, class Body, class Fields>
    void copy_headers(
        const http::message<isRequest, Body, Fields>& source,
        http::message<!isRequest, http::string_body, http::basic_fields<std::allocator<char>>>& target
    );

private:
    beast::tcp_stream stream_;
    std::unique_ptr<beast::ssl_stream<beast::tcp_stream>> ssl_stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;
    std::shared_ptr<RequestRouter> router_;
    bool is_ssl_;
    
    // Backend connection
    std::unique_ptr<beast::tcp_stream> backend_stream_;
    beast::flat_buffer backend_buffer_;
    http::request<http::string_body> backend_req_;
    http::response<http::string_body> backend_res_;
    
    // WebSocket support
    std::unique_ptr<websocket::stream<beast::tcp_stream>> ws_stream_;
    std::unique_ptr<websocket::stream<beast::tcp_stream>> backend_ws_stream_;
};

#endif // CONNECTION_HANDLER_H
