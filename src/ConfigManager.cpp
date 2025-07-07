#include "ConfigManager.h"
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <filesystem>

std::shared_ptr<ConfigManager> ConfigManager::instance_ = nullptr;

std::shared_ptr<ConfigManager> ConfigManager::getInstance() {
    if (!instance_) {
        instance_ = std::shared_ptr<ConfigManager>(new ConfigManager());
    }
    return instance_;
}

bool ConfigManager::loadConfig(const std::string& configPath) {
    try {
        if (!std::filesystem::exists(configPath)) {
            std::cerr << "Config file not found: " << configPath << std::endl;
            return false;
        }

        YAML::Node config = YAML::LoadFile(configPath);
        
        // Load basic settings
        if (config["http_port"]) {
            config_.http_port = config["http_port"].as<int>();
        }
        
        if (config["https_port"]) {
            config_.https_port = config["https_port"].as<int>();
        }
        
        if (config["email"]) {
            config_.email = config["email"].as<std::string>();
        }
        
        if (config["timeout_seconds"]) {
            config_.timeout_seconds = config["timeout_seconds"].as<int>();
        }
        
        if (config["max_connections"]) {
            config_.max_connections = config["max_connections"].as<int>();
        }
        
        if (config["cert_dir"]) {
            config_.cert_dir = config["cert_dir"].as<std::string>();
        }
        
        if (config["acme_server"]) {
            config_.acme_server = config["acme_server"].as<std::string>();
        }
        
        // Load sites
        if (config["sites"]) {
            config_.sites.clear();
            for (const auto& site : config["sites"]) {
                SiteConfig siteConfig;
                siteConfig.domain = site["domain"].as<std::string>();
                siteConfig.backend = site["backend"].as<std::string>();
                siteConfig.tls = site["tls"] ? site["tls"].as<std::string>() : "off";
                siteConfig.websocket = site["websocket"] ? site["websocket"].as<bool>() : false;
                
                config_.sites.push_back(siteConfig);
            }
        }
        
        // Create cert directory if it doesn't exist
        std::filesystem::create_directories(config_.cert_dir);
        
        std::cout << "Configuration loaded successfully from: " << configPath << std::endl;
        std::cout << "Loaded " << config_.sites.size() << " site configurations" << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        return false;
    }
}

const SiteConfig* ConfigManager::findSiteByDomain(const std::string& domain) const {
    for (const auto& site : config_.sites) {
        if (site.domain == domain) {
            return &site;
        }
    }
    return nullptr;
}

bool ConfigManager::needsTLS(const std::string& domain) const {
    const SiteConfig* site = findSiteByDomain(domain);
    return site && (site->tls == "auto" || site->tls == "manual");
}
