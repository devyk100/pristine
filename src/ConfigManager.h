#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <vector>
#include <memory>

struct SiteConfig {
    std::string domain;
    std::string backend;
    std::string tls;  // "auto", "manual", or "off"
    bool websocket = false;
};

struct ProxyConfig {
    int http_port = 80;
    int https_port = 443;
    std::string email;
    int timeout_seconds = 30;
    int max_connections = 1000;
    std::vector<SiteConfig> sites;
    std::string cert_dir = "./certs";
    std::string acme_server = "https://acme-v02.api.letsencrypt.org/directory";
};

class ConfigManager {
public:
    static std::shared_ptr<ConfigManager> getInstance();
    bool loadConfig(const std::string& configPath);
    const ProxyConfig& getConfig() const { return config_; }
    
    // Helper methods
    const SiteConfig* findSiteByDomain(const std::string& domain) const;
    bool needsTLS(const std::string& domain) const;

private:
    ConfigManager() = default;
    static std::shared_ptr<ConfigManager> instance_;
    ProxyConfig config_;
};

#endif // CONFIG_MANAGER_H
