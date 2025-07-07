#include "ConnectionHandler.h"
#include <iostream>
#include <boost/asio/connect.hpp>

ConnectionHandler::ConnectionHandler(
    tcp::socket&& socket,
    std::shared_ptr<RequestRouter> router,
    bool is_ssl
) : stream_(std::move(socket)), router_(router), is_ssl_(is_ssl) {
}

ConnectionHandler::ConnectionHandler(
    beast::ssl_stream<beast::tcp_stream>&& ssl_socket,
    std::shared_ptr<RequestRouter> router
) : stream_(ssl_socket.next_layer().release_socket()),
    ssl_stream_(std::make_unique<beast::ssl_stream<beast::tcp_stream>>(std::move(ssl_socket))),
    router_(router), is_ssl_(true) {
}

void ConnectionHandler::start() {
    do_read();
}

void ConnectionHandler::do_read() {
    req_ = {};
    
    if (is_ssl_ && ssl_stream_) {
        http::async_read(*ssl_stream_, buffer_, req_,
            beast::bind_front_handler(&ConnectionHandler::on_read, shared_from_this()));
    } else {
        http::async_read(stream_, buffer_, req_,
            beast::bind_front_handler(&ConnectionHandler::on_read, shared_from_this()));
    }
}

void ConnectionHandler::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    
    if (ec == http::error::end_of_stream) {
        close_connection();
        return;
    }
    
    if (ec) {
        std::cerr << "Read error: " << ec.message() << std::endl;
        return;
    }
    
    handle_request();
}

void ConnectionHandler::handle_request() {
    std::string host = extract_host_from_request();
    if (host.empty()) {
        send_error_response(http::status::bad_request, "Missing Host header");
        return;
    }
    
    // Check if this is a WebSocket upgrade request
    if (websocket::is_upgrade(req_) && router_->isWebSocketEnabled(host)) {
        handle_websocket_upgrade();
        return;
    }
    
    forward_to_backend();
}

void ConnectionHandler::forward_to_backend() {
    std::string host = extract_host_from_request();
    auto [backend_host, backend_port] = router_->getBackendForDomain(host);
    
    if (backend_host.empty() || backend_port == 0) {
        send_error_response(http::status::not_found, "No backend configured for domain");
        return;
    }
    
    // Create backend connection
    backend_stream_ = std::make_unique<beast::tcp_stream>(stream_.get_executor());
    
    // Resolve backend address
    tcp::resolver resolver(stream_.get_executor());
    auto const results = resolver.resolve(backend_host, std::to_string(backend_port));
    
    // Connect to backend
    backend_stream_->async_connect(results,
        [self = shared_from_this()](beast::error_code ec, tcp::endpoint) {
            self->on_backend_connect(ec);
        });
}

void ConnectionHandler::on_backend_connect(beast::error_code ec) {
    if (ec) {
        std::cerr << "Backend connect error: " << ec.message() << std::endl;
        send_error_response(http::status::bad_gateway, "Backend connection failed");
        return;
    }
    
    // Prepare request for backend (copy all headers)
    backend_req_ = req_;
    
    // Forward request to backend
    http::async_write(*backend_stream_, backend_req_,
        beast::bind_front_handler(&ConnectionHandler::on_backend_write, shared_from_this()));
}

void ConnectionHandler::on_backend_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    
    if (ec) {
        std::cerr << "Backend write error: " << ec.message() << std::endl;
        send_error_response(http::status::bad_gateway, "Backend write failed");
        return;
    }
    
    // Read response from backend
    backend_res_ = {};
    http::async_read(*backend_stream_, backend_buffer_, backend_res_,
        beast::bind_front_handler(&ConnectionHandler::on_backend_read, shared_from_this()));
}

void ConnectionHandler::on_backend_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    
    if (ec) {
        std::cerr << "Backend read error: " << ec.message() << std::endl;
        send_error_response(http::status::bad_gateway, "Backend read failed");
        return;
    }
    
    // Forward response to client
    res_ = std::move(backend_res_);
    
    if (is_ssl_ && ssl_stream_) {
        http::async_write(*ssl_stream_, res_,
            [self = shared_from_this()](beast::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "Client write error: " << ec.message() << std::endl;
                }
                self->close_connection();
            });
    } else {
        http::async_write(stream_, res_,
            [self = shared_from_this()](beast::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "Client write error: " << ec.message() << std::endl;
                }
                self->close_connection();
            });
    }
}

void ConnectionHandler::handle_websocket_upgrade() {
    std::string host = extract_host_from_request();
    auto [backend_host, backend_port] = router_->getBackendForDomain(host);
    
    if (backend_host.empty() || backend_port == 0) {
        send_error_response(http::status::not_found, "No backend configured for WebSocket");
        return;
    }
    
    // Create WebSocket streams
    if (is_ssl_ && ssl_stream_) {
        ws_stream_ = std::make_unique<websocket::stream<beast::tcp_stream>>(std::move(stream_));
    } else {
        ws_stream_ = std::make_unique<websocket::stream<beast::tcp_stream>>(std::move(stream_));
    }
    
    backend_ws_stream_ = std::make_unique<websocket::stream<beast::tcp_stream>>(
        beast::tcp_stream(stream_.get_executor()));
    
    // Connect to backend and upgrade both connections
    tcp::resolver resolver(stream_.get_executor());
    auto const results = resolver.resolve(backend_host, std::to_string(backend_port));
    
    boost::asio::async_connect(backend_ws_stream_->next_layer().socket(), results,
        [self = shared_from_this()](beast::error_code ec, tcp::endpoint) {
            if (ec) {
                std::cerr << "WebSocket backend connect error: " << ec.message() << std::endl;
                return;
            }
            
            // Accept WebSocket on client side and connect to backend
            self->ws_stream_->async_accept(self->req_,
                [self](beast::error_code ec) {
                    if (ec) {
                        std::cerr << "WebSocket accept error: " << ec.message() << std::endl;
                        return;
                    }
                    
                    // Upgrade backend connection to WebSocket
                    self->backend_ws_stream_->async_handshake(
                        self->extract_host_from_request(),
                        self->req_.target().to_string(),
                        [self](beast::error_code ec) {
                            if (ec) {
                                std::cerr << "WebSocket backend handshake error: " << ec.message() << std::endl;
                                return;
                            }
                            
                            // Start bidirectional forwarding
                            // This is a simplified implementation
                            // In production, you'd want proper bidirectional forwarding
                            std::cout << "WebSocket connection established" << std::endl;
                        });
                });
        });
}

void ConnectionHandler::send_error_response(http::status status, const std::string& message) {
    res_ = {};
    res_.result(status);
    res_.version(req_.version());
    res_.set(http::field::server, "ReverseProxy/1.0");
    res_.set(http::field::content_type, "text/plain");
    res_.body() = message;
    res_.prepare_payload();
    
    if (is_ssl_ && ssl_stream_) {
        http::async_write(*ssl_stream_, res_,
            [self = shared_from_this()](beast::error_code ec, std::size_t) {
                self->close_connection();
            });
    } else {
        http::async_write(stream_, res_,
            [self = shared_from_this()](beast::error_code ec, std::size_t) {
                self->close_connection();
            });
    }
}

void ConnectionHandler::close_connection() {
    beast::error_code ec;
    
    if (is_ssl_ && ssl_stream_) {
        ssl_stream_->next_layer().socket().shutdown(tcp::socket::shutdown_send, ec);
    } else {
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
    }
    
    if (backend_stream_) {
        backend_stream_->socket().shutdown(tcp::socket::shutdown_send, ec);
    }
}

std::string ConnectionHandler::extract_host_from_request() {
    auto host_it = req_.find(http::field::host);
    if (host_it != req_.end()) {
        std::string host = host_it->value().to_string();
        // Remove port if present
        size_t colon_pos = host.find(':');
        if (colon_pos != std::string::npos) {
            host = host.substr(0, colon_pos);
        }
        return host;
    }
    return "";
}

template<bool isRequest, class Body, class Fields>
void ConnectionHandler::copy_headers(
    const http::message<isRequest, Body, Fields>& source,
    http::message<!isRequest, http::string_body, http::basic_fields<std::allocator<char>>>& target
) {
    for (auto const& field : source) {
        target.set(field.name(), field.value());
    }
}
