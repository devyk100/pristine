#include "RequestRouter.h"
#include <sstream>
#include <iostream>

RequestRouter::RequestRouter(std::shared_ptr<ConfigManager> configManager)
    : configManager_(configManager) {
}

std::pair<std::string, int> RequestRouter::getBackendForDomain(const std::string& domain) const {
    const SiteConfig* site = configManager_->findSiteByDomain(domain);
    if (!site) {
        std::cerr << "No backend configured for domain: " << domain << std::endl;
        return {"", 0};
    }
    
    return parseBackendAddress(site->backend);
}

bool RequestRouter::isWebSocketEnabled(const std::string& domain) const {
    const SiteConfig* site = configManager_->findSiteByDomain(domain);
    return site && site->websocket;
}

bool RequestRouter::requiresTLS(const std::string& domain) const {
    return configManager_->needsTLS(domain);
}

std::pair<std::string, int> RequestRouter::parseBackendAddress(const std::string& backend) const {
    size_t colonPos = backend.find_last_of(':');
    if (colonPos == std::string::npos) {
        std::cerr << "Invalid backend format (expected host:port): " << backend << std::endl;
        return {"", 0};
    }
    
    std::string host = backend.substr(0, colonPos);
    std::string portStr = backend.substr(colonPos + 1);
    
    try {
        int port = std::stoi(portStr);
        return {host, port};
    } catch (const std::exception& e) {
        std::cerr << "Invalid port in backend: " << backend << std::endl;
        return {"", 0};
    }
}
