#ifndef REQUEST_ROUTER_H
#define REQUEST_ROUTER_H

#include "ConfigManager.h"
#include <string>
#include <memory>

class RequestRouter {
public:
    RequestRouter(std::shared_ptr<ConfigManager> configManager);
    
    // Parse backend address from domain
    std::pair<std::string, int> getBackendForDomain(const std::string& domain) const;
    
    // Check if domain supports WebSocket
    bool isWebSocketEnabled(const std::string& domain) const;
    
    // Check if domain needs TLS
    bool requiresTLS(const std::string& domain) const;

private:
    std::shared_ptr<ConfigManager> configManager_;
    
    // Helper to parse "host:port" format
    std::pair<std::string, int> parseBackendAddress(const std::string& backend) const;
};

#endif // REQUEST_ROUTER_H
