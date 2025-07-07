#include "ReverseProxy.h"
#include <iostream>
#include <signal.h>

ReverseProxy* g_proxy = nullptr;

void signal_handler(int signal) {
    if (g_proxy) {
        std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
        g_proxy->stop();
    }
}

int main(int argc, char* argv[]) {
    std::string config_file = "config/proxy.yaml";
    
    if (argc > 1) {
        config_file = argv[1];
    }
    
    try {
        // Create reverse proxy
        ReverseProxy proxy;
        g_proxy = &proxy;
        
        // Setup signal handlers
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        
        // Initialize with configuration
        if (!proxy.initialize(config_file)) {
            std::cerr << "Failed to initialize reverse proxy with config: " << config_file << std::endl;
            return 1;
        }
        
        std::cout << "Reverse proxy initialized successfully" << std::endl;
        std::cout << "Starting reverse proxy..." << std::endl;
        
        // Run the proxy (this will block until stopped)
        proxy.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Reverse proxy shutdown complete" << std::endl;
    return 0;
}
