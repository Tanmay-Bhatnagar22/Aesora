/**
 * @file crypto_utils.cpp
 * @brief Implementation of cryptographic utilities
 * 
 * SECURITY DESIGN:
 * ================
 * This implementation uses OpenSSL's high-level EVP interface for:
 * - PBKDF2-HMAC-SHA256 key derivation
 * - AES-256-GCM authenticated encryption
 * - Secure random number generation
 * 
 * WHY THESE CHOICES:
 * - PBKDF2 is NIST-approved and well-studied
 * - 310,000 iterations makes each password guess ~300ms (impractical for brute-force)
 * - AES-256-GCM is faster and more secure than AES-256-CBC with separate HMAC
 * - EVP interface handles all cryptographic details correctly
 * 
 * THREATS MITIGATED:
 * 1. Brute-force attacks: High PBKDF2 iteration count
 * 2. Rainbow tables: Random unique salt per password
 * 3. Tampering: GCM authentication tag
 * 4. Patterns: Random IV for each encryption
 * 5. Predictable keys: Secure random generation with RAND_bytes
 */

#include "crypto_utils.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <cstring>
#include <stdexcept>
#include <iostream>

/**
 * Helper function to get detailed OpenSSL error messages.
 * Useful for debugging cryptographic operations.
 */
static std::string get_openssl_error() {
    unsigned long error = ERR_get_error();
    if (error == 0) return "Unknown error";
    char error_buffer[256] = {0};
    ERR_error_string_n(error, error_buffer, sizeof(error_buffer));
    return std::string(error_buffer);
}

std::vector<unsigned char> generate_random_bytes(size_t length) {
    std::vector<unsigned char> buffer(length);
    
    // RAND_bytes returns 1 on success, 0 on failure
    // On failure, the random buffer is not suitable for cryptographic use
    if (RAND_bytes(buffer.data(), static_cast<int>(length)) != 1) {
        throw std::runtime_error(
            "Secure random generation failed. This usually means the system "
            "entropy pool is depleted. Details: " + get_openssl_error()
        );
    }
    
    return buffer;
}

std::vector<unsigned char> derive_key_from_password(
    const std::string& password,
    const std::vector<unsigned char>& salt,
    int iterations
) {
    // Output key will be 32 bytes (256 bits) for AES-256
    const int KEY_LENGTH = 32;
    std::vector<unsigned char> key(KEY_LENGTH);
    
    // WHY PBKDF2:
    // PBKDF2 is a key derivation function approved by NIST.
    // It prevents brute-force attacks by making each password attempt expensive.
    // 
    // WHY PBKDF2-HMAC-SHA256:
    // - SHA256 is cryptographically strong and collision-resistant
    // - HMAC prevents key recovery even if PRF is broken
    // - Well-studied and widely trusted
    // 
    // WHY 310,000 ITERATIONS:
    // - Each iteration takes ~1 microsecond on modern CPU
    // - 310,000 iterations = ~310ms per password guess
    // - Makes offline brute-force attack on 10-char password impractical
    // - NIST recommends at least 1,000 iterations (we use 310x more)
    // - Can be increased further as hardware gets faster
    
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),                    // Password input
        static_cast<int>(password.length()), // Password length
        salt.data(),                         // Salt input
        static_cast<int>(salt.size()),       // Salt length
        iterations,                          // Iteration count
        EVP_sha256(),                        // Hash function (SHA256)
        KEY_LENGTH,                          // Desired key length
        key.data()                           // Output buffer
    );
    
    if (result != 1) {
        throw std::runtime_error(
            "PBKDF2 key derivation failed: " + get_openssl_error()
        );
    }
    
    return key;
}

std::vector<unsigned char> encrypt_aes256_gcm(
    const std::vector<unsigned char>& plaintext,
    const std::vector<unsigned char>& key,
    const std::vector<unsigned char>& iv
) {
    // WHY AES-256-GCM:
    // AES-GCM (Galois/Counter Mode) provides authenticated encryption:
    // - Encrypts data (confidentiality via AES-256)
    // - Generates authentication tag (integrity via GCM)
    // - Single-pass algorithm (faster than CBC + HMAC)
    // - Provably secure (NIST-approved)
    // - Prevents:
    //   a) Confidentiality breach (via AES-256)
    //   b) Integrity violation (via GCM tag)
    //   c) Padding oracle attacks (no padding needed)
    //
    // WHY 12-BYTE IV:
    // - 12 bytes (96 bits) is the "sweet spot" for GCM
    // - Longer IVs require hashing, adding computational cost
    // - 96 bits provides ~2^48 encryptions before birthday attack risk
    // - MUST be random and never reused with same key
    
    if (key.size() != 32) {
        throw std::runtime_error("Invalid AES-256 key size (must be 32 bytes)");
    }
    if (iv.size() != 12) {
        throw std::runtime_error("Invalid GCM IV size (must be 12 bytes)");
    }
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context");
    }
    
    std::vector<unsigned char> ciphertext(plaintext.size() + 16); // +16 for tag
    int len = 0;
    int ciphertext_len = 0;
    
    try {
        // Initialize encryption context
        // EVP_aes_256_gcm() returns AES-256 in GCM mode
        if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), iv.data())) {
            throw std::runtime_error("Encryption initialization failed: " + get_openssl_error());
        }
        
        // Encrypt plaintext
        // Note: For GCM, additional data (AAD) is not used in Aesora
        // All data is encrypted (salt and IV are stored plaintext, not encrypted)
        if (1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size())) {
            throw std::runtime_error("Encryption failed: " + get_openssl_error());
        }
        ciphertext_len = len;
        
        // Finalize encryption (handles any remaining data)
        if (1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) {
            throw std::runtime_error("Encryption finalization failed: " + get_openssl_error());
        }
        ciphertext_len += len;
        
        // Extract and append authentication tag (16 bytes)
        // WHY APPEND TAG:
        // - Makes decryption streaming-friendly (tag at end)
        // - Decryption can consume data as it arrives
        // - Tag is not needed until end of data
        unsigned char tag[16] = {0};
        if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag)) {
            throw std::runtime_error("Failed to get authentication tag: " + get_openssl_error());
        }
        
        // Append tag to ciphertext
        std::memcpy(ciphertext.data() + ciphertext_len, tag, 16);
        ciphertext_len += 16;
        
        // Resize to actual size (ciphertext.size() might include extra space)
        ciphertext.resize(ciphertext_len);
        
    } catch (...) {
        EVP_CIPHER_CTX_free(ctx);
        throw;
    }
    
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

std::vector<unsigned char> decrypt_aes256_gcm(
    const std::vector<unsigned char>& ciphertext_with_tag,
    const std::vector<unsigned char>& key,
    const std::vector<unsigned char>& iv
) {
    // SECURITY: Verify tag at decryption time
    // WHY AUTHENTICATE:
    // - Ensures ciphertext wasn't modified (maliciously or accidentally)
    // - Returns empty if tag fails (never returns untrusted plaintext)
    // - Prevents attackers from flipping bits in ciphertext
    // 
    // WHY THIS MATTERS:
    // - Without authentication, attacker could flip bits in ciphertext
    // - Bits would decrypt to garbage, but attacker could potentially
    //   cause decryption to do something unintended
    // - GCM tag prevents this entirely
    
    if (key.size() != 32) {
        throw std::runtime_error("Invalid AES-256 key size (must be 32 bytes)");
    }
    if (iv.size() != 12) {
        throw std::runtime_error("Invalid GCM IV size (must be 12 bytes)");
    }
    
    // GCM tag is 16 bytes, must be present
    if (ciphertext_with_tag.size() < 16) {
        throw std::runtime_error("Ciphertext too short (missing authentication tag)");
    }
    
    // Split tag from ciphertext
    size_t ct_len = ciphertext_with_tag.size() - 16;
    const unsigned char* ct_data = ciphertext_with_tag.data();
    const unsigned char* tag_data = ciphertext_with_tag.data() + ct_len;
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context");
    }
    
    std::vector<unsigned char> plaintext(ct_len);
    int len = 0;
    int plaintext_len = 0;
    
    try {
        // Initialize decryption context
        if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), iv.data())) {
            throw std::runtime_error("Decryption initialization failed: " + get_openssl_error());
        }
        
        // Set authentication tag BEFORE decryption
        // This tells OpenSSL to verify the tag after decryption
        if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, const_cast<unsigned char*>(tag_data))) {
            throw std::runtime_error("Failed to set authentication tag: " + get_openssl_error());
        }
        
        // Decrypt ciphertext
        if (1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, ct_data, ct_len)) {
            throw std::runtime_error("Decryption failed: " + get_openssl_error());
        }
        plaintext_len = len;
        
        // Finalize decryption and verify tag
        // CRITICAL: EVP_DecryptFinal_ex returns 0 if tag verification fails
        // This is the primary security check - if tag doesn't match, plaintext is rejected
        int result = EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);
        if (result != 1) {
            // Authentication tag verification failed
            // This indicates EITHER:
            // 1. Wrong password (different key, wrong tag)
            // 2. Ciphertext was corrupted or modified
            // We don't distinguish between these (good for security)
            throw std::runtime_error("Authentication tag verification failed. Possible causes: wrong password or file was corrupted/modified.");
        }
        plaintext_len += len;
        plaintext.resize(plaintext_len);
        
    } catch (...) {
        EVP_CIPHER_CTX_free(ctx);
        throw;
    }
    
    EVP_CIPHER_CTX_free(ctx);
    return plaintext;
}

void clear_sensitive_data(std::vector<unsigned char>& data) {
    // SECURITY: Prevent compiler from optimizing away memset
    // WHY NEEDED:
    // - Modern compilers optimize away memset() on dead variables
    // - This leaves sensitive data in RAM after use
    // - cold-boot attacks or memory dumps could recover passwords/keys
    // 
    // TECHNIQUE:
    // - Use volatile pointer to prevent optimization
    // - Force the compiler to write the zeros even if variable dies next
    // - Not perfect (determined attackers with physical access may recover data)
    // - But provides defense-in-depth against basic recovery
    
    if (data.empty()) return;
    
    volatile unsigned char* vdata = data.data();
    for (size_t i = 0; i < data.size(); ++i) {
        vdata[i] = 0;
    }
    
    // Also clear the vector metadata if possible
    // (Though this is less critical than clearing actual data)
    data.clear();
    data.shrink_to_fit();
}
