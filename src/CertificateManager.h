#ifndef CERTIFICATE_MANAGER_H
#define CERTIFICATE_MANAGER_H

#include <string>
#include <memory>
#include <map>
#include <boost/asio/ssl/context.hpp>

struct CertificateInfo {
    std::string cert_path;
    std::string key_path;
    std::string domain;
    time_t expiry_time;
    bool auto_renew;
};

class CertificateManager {
public:
    CertificateManager(const std::string& cert_dir, const std::string& email);
    
    // Load or generate certificate for domain
    bool ensure_certificate(const std::string& domain);
    
    // Get certificate paths for domain
    CertificateInfo get_certificate_info(const std::string& domain) const;
    
    // Setup SSL context with certificates
    void setup_ssl_context(boost::asio::ssl::context& ctx, const std::string& domain);
    
    // Check and renew certificates
    void check_renewals();

private:
    // ACME/Let's Encrypt integration
    bool request_certificate_acme(const std::string& domain);
    
    // Generate self-signed certificate for development
    bool generate_self_signed(const std::string& domain);
    
    // File operations
    bool certificate_exists(const std::string& domain) const;
    bool is_certificate_valid(const std::string& domain) const;
    std::string get_cert_path(const std::string& domain) const;
    std::string get_key_path(const std::string& domain) const;

private:
    std::string cert_dir_;
    std::string email_;
    std::map<std::string, CertificateInfo> certificates_;
};

#endif // CERTIFICATE_MANAGER_H
