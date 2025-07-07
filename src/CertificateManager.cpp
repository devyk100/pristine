#include "CertificateManager.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/x509v3.h>

CertificateManager::CertificateManager(const std::string& cert_dir, const std::string& email)
    : cert_dir_(cert_dir), email_(email) {
    
    // Create certificate directory if it doesn't exist
    std::filesystem::create_directories(cert_dir_);
    
    std::cout << "Certificate manager initialized with directory: " << cert_dir_ << std::endl;
}

bool CertificateManager::ensure_certificate(const std::string& domain) {
    if (certificate_exists(domain) && is_certificate_valid(domain)) {
        std::cout << "Valid certificate already exists for domain: " << domain << std::endl;
        return true;
    }
    
    std::cout << "Generating certificate for domain: " << domain << std::endl;
    
    // For now, generate self-signed certificates
    // In production, you would implement ACME protocol here
    return generate_self_signed(domain);
}

CertificateInfo CertificateManager::get_certificate_info(const std::string& domain) const {
    auto it = certificates_.find(domain);
    if (it != certificates_.end()) {
        return it->second;
    }
    
    CertificateInfo info;
    info.domain = domain;
    info.cert_path = get_cert_path(domain);
    info.key_path = get_key_path(domain);
    info.auto_renew = true;
    info.expiry_time = 0; // Will be set when certificate is loaded
    
    return info;
}

void CertificateManager::setup_ssl_context(boost::asio::ssl::context& ctx, const std::string& domain) {
    try {
        std::string cert_path = get_cert_path(domain);
        std::string key_path = get_key_path(domain);
        
        if (!std::filesystem::exists(cert_path) || !std::filesystem::exists(key_path)) {
            if (!ensure_certificate(domain)) {
                throw std::runtime_error("Failed to ensure certificate for domain: " + domain);
            }
        }
        
        ctx.use_certificate_chain_file(cert_path);
        ctx.use_private_key_file(key_path, boost::asio::ssl::context::pem);
        
        std::cout << "SSL context configured for domain: " << domain << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error setting up SSL context for " << domain << ": " << e.what() << std::endl;
        throw;
    }
}

void CertificateManager::check_renewals() {
    // Check all certificates for renewal
    for (const auto& [domain, info] : certificates_) {
        if (info.auto_renew && !is_certificate_valid(domain)) {
            std::cout << "Renewing certificate for domain: " << domain << std::endl;
            ensure_certificate(domain);
        }
    }
}

bool CertificateManager::request_certificate_acme(const std::string& domain) {
    // Placeholder for ACME implementation
    // This would implement the ACME protocol to get certificates from Let's Encrypt
    std::cout << "ACME certificate request not implemented yet for domain: " << domain << std::endl;
    return false;
}

bool CertificateManager::generate_self_signed(const std::string& domain) {
    try {
        // Generate RSA key pair
        EVP_PKEY* pkey = EVP_PKEY_new();
        RSA* rsa = RSA_new();
        BIGNUM* bne = BN_new();
        
        if (BN_set_word(bne, RSA_F4) != 1) {
            throw std::runtime_error("Failed to set RSA exponent");
        }
        
        if (RSA_generate_key_ex(rsa, 2048, bne, nullptr) != 1) {
            throw std::runtime_error("Failed to generate RSA key");
        }
        
        EVP_PKEY_assign_RSA(pkey, rsa);
        
        // Create certificate
        X509* x509 = X509_new();
        X509_set_version(x509, 2);
        ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
        X509_gmtime_adj(X509_get_notBefore(x509), 0);
        X509_gmtime_adj(X509_get_notAfter(x509), 365 * 24 * 3600); // 1 year
        
        X509_set_pubkey(x509, pkey);
        
        // Set subject and issuer
        X509_NAME* name = X509_get_subject_name(x509);
        X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char*)"US", -1, -1, 0);
        X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char*)"ReverseProxy", -1, -1, 0);
        X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)domain.c_str(), -1, -1, 0);
        X509_set_issuer_name(x509, name);
        
        // Add SAN extension
        X509_EXTENSION* ext = nullptr;
        X509V3_CTX ctx;
        X509V3_set_ctx_nodb(&ctx);
        X509V3_set_ctx(&ctx, x509, x509, nullptr, nullptr, 0);
        
        std::string san = "DNS:" + domain;
        ext = X509V3_EXT_conf_nid(nullptr, &ctx, NID_subject_alt_name, san.c_str());
        if (ext) {
            X509_add_ext(x509, ext, -1);
            X509_EXTENSION_free(ext);
        }
        
        // Sign certificate
        X509_sign(x509, pkey, EVP_sha256());
        
        // Write certificate to file
        std::string cert_path = get_cert_path(domain);
        FILE* cert_file = fopen(cert_path.c_str(), "wb");
        if (!cert_file) {
            throw std::runtime_error("Failed to open certificate file for writing");
        }
        PEM_write_X509(cert_file, x509);
        fclose(cert_file);
        
        // Write private key to file
        std::string key_path = get_key_path(domain);
        FILE* key_file = fopen(key_path.c_str(), "wb");
        if (!key_file) {
            throw std::runtime_error("Failed to open key file for writing");
        }
        PEM_write_PrivateKey(key_file, pkey, nullptr, nullptr, 0, nullptr, nullptr);
        fclose(key_file);
        
        // Cleanup
        X509_free(x509);
        EVP_PKEY_free(pkey);
        BN_free(bne);
        
        std::cout << "Generated self-signed certificate for domain: " << domain << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error generating self-signed certificate: " << e.what() << std::endl;
        return false;
    }
}

bool CertificateManager::certificate_exists(const std::string& domain) const {
    return std::filesystem::exists(get_cert_path(domain)) && 
           std::filesystem::exists(get_key_path(domain));
}

bool CertificateManager::is_certificate_valid(const std::string& domain) const {
    if (!certificate_exists(domain)) {
        return false;
    }
    
    // For now, just check if files exist
    // In production, you would check expiry date
    return true;
}

std::string CertificateManager::get_cert_path(const std::string& domain) const {
    return cert_dir_ + "/" + domain + ".crt";
}

std::string CertificateManager::get_key_path(const std::string& domain) const {
    return cert_dir_ + "/" + domain + ".key";
}
