#include "ReverseProxy.h"
#include <iostream>
#include <signal.h>

ReverseProxy::ReverseProxy() : running_(false) {
}

ReverseProxy::~ReverseProxy() {
    stop();
}

bool ReverseProxy::initialize(const std::string& configPath) {
    try {
        // Initialize configuration manager
        config_manager_ = ConfigManager::getInstance();
        if (!config_manager_->loadConfig(configPath)) {
            std::cerr << "Failed to load configuration from: " << configPath << std::endl;
            return false;
        }
        
        const auto& config = config_manager_->getConfig();
        
        // Initialize request router
        router_ = std::make_shared<RequestRouter>(config_manager_);
        
        // Initialize certificate manager
        cert_manager_ = std::make_shared<CertificateManager>(config.cert_dir, config.email);
        
        // Setup HTTP acceptor
        http_acceptor_ = std::make_unique<tcp::acceptor>(ioc_, tcp::endpoint(tcp::v4(), config.http_port));
        
        // Setup HTTPS acceptor if needed
        bool needs_https = false;
        for (const auto& site : config.sites) {
            if (site.tls == "auto" || site.tls == "manual") {
                needs_https = true;
                break;
            }
        }
        
        if (needs_https) {
            https_acceptor_ = std::make_unique<tcp::acceptor>(ioc_, tcp::endpoint(tcp::v4(), config.https_port));
            setup_ssl_context();
        }
        
        std::cout << "Reverse proxy initialized successfully" << std::endl;
        std::cout << "HTTP server listening on port: " << config.http_port << std::endl;
        if (needs_https) {
            std::cout << "HTTPS server listening on port: " << config.https_port << std::endl;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error initializing reverse proxy: " << e.what() << std::endl;
        return false;
    }
}

void ReverseProxy::run() {
    if (running_) {
        return;
    }
    
    running_ = true;
    
    // Start accepting connections
    start_http_server();
    if (https_acceptor_) {
        start_https_server();
    }
    
    // Create worker threads
    const auto& config = config_manager_->getConfig();
    int thread_count = std::max(1, static_cast<int>(std::thread::hardware_concurrency()));
    
    std::cout << "Starting " << thread_count << " worker threads" << std::endl;
    
    threads_.reserve(thread_count);
    for (int i = 0; i < thread_count; ++i) {
        threads_.emplace_back([this] {
            ioc_.run();
        });
    }
    
    std::cout << "Reverse proxy is running..." << std::endl;
    
    // Wait for all threads to complete
    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void ReverseProxy::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    std::cout << "Stopping reverse proxy..." << std::endl;
    
    // Stop acceptors
    if (http_acceptor_) {
        http_acceptor_->close();
    }
    if (https_acceptor_) {
        https_acceptor_->close();
    }
    
    // Stop IO context
    ioc_.stop();
    
    // Wait for threads to finish
    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    threads_.clear();
    
    std::cout << "Reverse proxy stopped" << std::endl;
}

void ReverseProxy::start_http_server() {
    accept_http_connections();
}

void ReverseProxy::start_https_server() {
    accept_https_connections();
}

void ReverseProxy::accept_http_connections() {
    http_acceptor_->async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            on_http_accept(ec, std::move(socket));
        });
}

void ReverseProxy::accept_https_connections() {
    https_acceptor_->async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            on_https_accept(ec, std::move(socket));
        });
}

void ReverseProxy::on_http_accept(beast::error_code ec, tcp::socket socket) {
    if (ec) {
        std::cerr << "HTTP accept error: " << ec.message() << std::endl;
    } else {
        // Create connection handler for HTTP
        auto handler = std::make_shared<ConnectionHandler>(std::move(socket), router_, false);
        handler->start();
    }
    
    // Continue accepting connections
    if (running_) {
        accept_http_connections();
    }
}

void ReverseProxy::on_https_accept(beast::error_code ec, tcp::socket socket) {
    if (ec) {
        std::cerr << "HTTPS accept error: " << ec.message() << std::endl;
    } else {
        // Create SSL stream and connection handler for HTTPS
        beast::ssl_stream<beast::tcp_stream> ssl_stream(std::move(socket), *ssl_ctx_);
        
        // Perform SSL handshake
        ssl_stream.async_handshake(ssl::stream_base::server,
            [this, ssl_stream = std::move(ssl_stream)](beast::error_code ec) mutable {
                if (ec) {
                    std::cerr << "SSL handshake error: " << ec.message() << std::endl;
                    return;
                }
                
                auto handler = std::make_shared<ConnectionHandler>(std::move(ssl_stream), router_);
                handler->start();
            });
    }
    
    // Continue accepting connections
    if (running_) {
        accept_https_connections();
    }
}

void ReverseProxy::setup_ssl_context() {
    ssl_ctx_ = std::make_unique<ssl::context>(ssl::context::tlsv12);
    
    ssl_ctx_->set_options(
        ssl::context::default_workarounds |
        ssl::context::no_sslv2 |
        ssl::context::no_sslv3 |
        ssl::context::single_dh_use);
    
    // For now, use a default certificate
    // In production, you would set up SNI callback to select appropriate certificate
    const auto& config = config_manager_->getConfig();
    if (!config.sites.empty()) {
        const auto& first_site = config.sites[0];
        if (first_site.tls == "auto" || first_site.tls == "manual") {
            try {
                cert_manager_->setup_ssl_context(*ssl_ctx_, first_site.domain);
            } catch (const std::exception& e) {
                std::cerr << "Warning: Failed to setup SSL context: " << e.what() << std::endl;
            }
        }
    }
}
