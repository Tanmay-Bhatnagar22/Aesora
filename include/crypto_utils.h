/**
 * @file crypto_utils.h
 * @brief Cryptographic utilities for Aesora
 * 
 * This module provides core cryptographic operations:
 * - Key derivation from passwords (PBKDF2-HMAC-SHA256)
 * - Secure encryption with AES-256-GCM
 * - Secure decryption with authentication verification
 * 
 * SECURITY DESIGN:
 * ================
 * - Uses PBKDF2-HMAC-SHA256 with 310,000 iterations to resist brute-force attacks
 * - AES-256-GCM provides authenticated encryption (confidentiality + integrity)
 * - Random salt (16 bytes) and IV (12 bytes) generated per encryption
 * - Authentication tag verified during decryption to detect tampering
 * - All sensitive data should be cleared from memory after use
 */

#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <vector>
#include <string>
#include <cstdint>

/**
 * Derives a 256-bit key from a password using PBKDF2-HMAC-SHA256.
 * 
 * PURPOSE:
 * Converts a user's password into a cryptographic key suitable for AES-256.
 * Uses a salt to prevent rainbow table attacks and high iteration count to resist brute-force.
 * 
 * SECURITY CONSIDERATIONS:
 * - Salt MUST be cryptographically random and unique per password
 * - Iteration count (310,000) makes brute-force impractical (~1ms per attempt)
 * - PBKDF2-HMAC-SHA256 is NIST-approved and widely trusted
 * - Output should be cleared from memory after use (see clear_sensitive_data)
 * 
 * @param password      User's plaintext password (can contain any bytes)
 * @param salt          16-byte cryptographic salt (should be random)
 * @param iterations    Number of PBKDF2 iterations (default 310,000 recommended)
 * @return              32-byte key suitable for AES-256
 * @throws              std::runtime_error if OpenSSL operations fail
 */
std::vector<unsigned char> derive_key_from_password(
    const std::string& password,
    const std::vector<unsigned char>& salt,
    int iterations = 310000
);

/**
 * Generates cryptographically secure random bytes.
 * 
 * PURPOSE:
 * Generates salt and IV using OpenSSL's secure PRNG, which is properly seeded
 * from the OS's entropy pool.
 * 
 * SECURITY CONSIDERATIONS:
 * - Uses OpenSSL's RAND_bytes, which is cryptographically secure
 * - For AES-GCM, IV MUST be random and never reused with same key
 * - Salt should be random and stored with ciphertext for decryption
 * - Output should be treated as sensitive and cleared when no longer needed
 * 
 * @param length        Number of random bytes to generate
 * @return              Vector of random bytes
 * @throws              std::runtime_error if random generation fails (entropy depleted, etc.)
 */
std::vector<unsigned char> generate_random_bytes(size_t length);

/**
 * Encrypts plaintext using AES-256-GCM.
 * 
 * PURPOSE:
 * Provides authenticated encryption that ensures both confidentiality (AES-256)
 * and integrity (GCM authentication tag). GCM is superior to CBC + HMAC because:
 * - Faster (single-pass authenticated encryption)
 * - Resistant to padding oracle attacks (no padding needed)
 * - Built-in authentication prevents tampering
 * 
 * PARAMETERS:
 * - plaintext:  Data to encrypt (any binary data, any length)
 * - key:        32-byte AES-256 key from key derivation
 * - iv:         12-byte nonce (initialization vector) for this encryption
 *               CRITICAL: Must be random and never reused with same key
 * 
 * SECURITY CONSIDERATIONS:
 * - AES-256-GCM prevents both:
 *   a) Confidentiality breach (thanks to AES-256)
 *   b) Integrity violation (thanks to GCM authentication)
 * - IV MUST be random for each encryption with the same key
 * - IV length must be exactly 12 bytes (96 bits) for optimal GCM performance
 * - GCM tag is included in output and verified during decryption
 * - Failure to verify tag during decryption indicates tampering
 * 
 * @param plaintext     Data to encrypt (binary-safe)
 * @param key          32-byte encryption key
 * @param iv           12-byte initialization vector (must be cryptographically random)
 * @return             Ciphertext + 16-byte GCM authentication tag
 * @throws             std::runtime_error if encryption fails
 */
std::vector<unsigned char> encrypt_aes256_gcm(
    const std::vector<unsigned char>& plaintext,
    const std::vector<unsigned char>& key,
    const std::vector<unsigned char>& iv
);

/**
 * Decrypts AES-256-GCM ciphertext and verifies authenticity.
 * 
 * PURPOSE:
 * Reverses encryption and cryptographically verifies the authentication tag.
 * If tag verification fails, the data was tampered with and is rejected.
 * 
 * PARAMETERS:
 * - ciphertext_with_tag: Ciphertext + 16-byte GCM tag (as produced by encrypt_aes256_gcm)
 * - key:                 Same 32-byte key used during encryption
 * - iv:                  Same 12-byte IV used during encryption
 * 
 * SECURITY CONSIDERATIONS:
 * - ALWAYS verifies GCM authentication tag (cryptographically authenticated decryption)
 * - If authentication fails, returns empty vector (no plaintext leakage)
 * - If tag is invalid, either:
 *   a) Ciphertext was corrupted during transmission/storage
 *   b) Ciphertext was intentionally modified by attacker
 *   c) Wrong password was used (different key, wrong tag verification)
 * - Constant-time comparison used for tag verification to prevent timing attacks
 * - Never returns plaintext if authentication fails
 * 
 * @param ciphertext_with_tag  Encrypted data + 16-byte GCM tag
 * @param key                 32-byte decryption key
 * @param iv                  12-byte nonce (same as encryption)
 * @return                    Plaintext if authentication succeeds, empty vector otherwise
 * @throws                    std::runtime_error if decryption fails (but not auth failure)
 */
std::vector<unsigned char> decrypt_aes256_gcm(
    const std::vector<unsigned char>& ciphertext_with_tag,
    const std::vector<unsigned char>& key,
    const std::vector<unsigned char>& iv
);

/**
 * Securely clears sensitive data from memory.
 * 
 * PURPOSE:
 * Overwrite sensitive data (keys, passwords, plaintexts) with zeros to prevent
 * recovery from memory dumps or swap files.
 * 
 * SECURITY CONSIDERATIONS:
 * - Prevents forensic recovery of sensitive data from RAM
 * - Compiler might optimize away memset() without volatile_memset, so we use
 *   a memory barrier technique
 * - Should be called on:
 *   a) Passwords after key derivation
 *   b) Derived keys after encryption/decryption
 *   c) Plaintext after encryption
 * - Note: This is defense-in-depth; determined attackers with physical access
 *   may still recover data through cold-boot attacks or side-channel analysis
 * 
 * @param data   Vector of sensitive data to clear
 */
void clear_sensitive_data(std::vector<unsigned char>& data);

#endif // CRYPTO_UTILS_H
