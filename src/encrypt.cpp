/**
 * @file encrypt.cpp
 * @brief Implementation of file encryption
 * 
 * This module orchestrates:
 * 1. Random salt and IV generation
 * 2. Key derivation from password
 * 3. AES-256-GCM encryption
 * 4. Aesora file format creation
 */

#include "encrypt.h"
#include "crypto_utils.h"
#include "file_utils.h"

#include <cstring>
#include <stdexcept>
#include <iostream>

void encrypt_file(
    const std::string& input_file,
    const std::string& output_file,
    const std::string& password
) {
    try {
        // Step 1: Read plaintext file
        // WHY BINARY READ:
        // - Works with any file type: text, images, archives, etc.
        // - No encoding conversion (binary-safe)
        std::vector<unsigned char> plaintext = read_file_binary(input_file);
        
        // Step 2: Generate random salt
        // WHY 16 BYTES:
        // - 128 bits of entropy
        // - Prevents rainbow table attacks (2^128 possible salts)
        // - Unique for each password prevents pre-computation
        // - Stored plaintext in encrypted file (salts don't need secrecy)
        // WHY RANDOM:
        // - Even identical passwords will generate different keys
        // - Makes offline brute-force exponentially harder
        std::vector<unsigned char> salt = generate_random_bytes(AesoraFileHeader::SALT_SIZE);
        
        // Step 3: Generate random IV
        // WHY 12 BYTES:
        // - Optimal for AES-GCM (96-bit IV)
        // - Sufficient uniqueness (2^48 encryptions before birthday attack risk)
        // WHY RANDOM:
        // - Same plaintext encrypted twice produces different ciphertexts
        // - Prevents pattern attacks
        // WHY NEVER REUSE:
        // - With same key, reused IV completely breaks GCM security
        // - Attacker can XOR two ciphertexts to get XOR of plaintexts
        // - We generate new random IV for each encryption (prevents this)
        std::vector<unsigned char> iv = generate_random_bytes(AesoraFileHeader::IV_SIZE);
        
        // Step 4: Derive encryption key from password
        // WHY PBKDF2:
        // - NIST-approved key derivation function
        // - High iteration count makes brute-force expensive
        // WHY SALT:
        // - Same password never produces same key twice (different salt)
        // - Forces attacker to crack each password separately
        // - Prevents using precomputed hash tables (rainbow tables)
        std::vector<unsigned char> key = derive_key_from_password(password, salt);
        
        // Step 5: Encrypt plaintext with AES-256-GCM
        // WHY GCM:
        // - Authenticated encryption: confidentiality + integrity
        // - Single-pass: faster than CBC + HMAC
        // - Prevents padding oracle attacks
        // CIPHERTEXT FORMAT:
        // - Output from encrypt_aes256_gcm is: [Ciphertext][16-byte GCM Tag]
        // - Tag is appended for convenient file format storage
        std::vector<unsigned char> ciphertext_with_tag = encrypt_aes256_gcm(plaintext, key, iv);
        
        // Clear plaintext from memory (defense-in-depth)
        // WHY CLEAR:
        // - Prevents sensitive data in RAM from being recovered
        // - Especially important on servers with memory dumps
        // - On desktop, less critical but still good practice
        clear_sensitive_data(plaintext);
        
        // Clear the key from memory (very important)
        // WHY:
        // - Key should never remain in memory after encryption
        // - If attacker has RAM access, key in memory = compromise
        // - After encryption, key is no longer needed
        clear_sensitive_data(key);
        
        // Step 6: Construct Aesora file format
        // FILE FORMAT:
        // [8-byte Magic] [16-byte Salt] [12-byte IV] [Ciphertext] [16-byte GCM Tag]
        // 
        // TOTAL SIZE:
        // = 8 + 16 + 12 + ciphertext.size()
        // = 36 + ciphertext.size()
        // 
        // For empty plaintext: 36 + 16 = 52 bytes
        // For 1MB plaintext: 36 + 1MB + 16 = 1MB + 52 bytes
        
        std::vector<unsigned char> encrypted_file;
        encrypted_file.reserve(36 + ciphertext_with_tag.size());
        
        // Add magic header
        // MAGIC NUMBER: 0x4145534F52410000
        // In ASCII: "AESORA\0\0" (null-terminated for safety)
        // PURPOSE:
        // - Identifies file as Aesora-encrypted
        // - Prevents accidentally decrypting non-Aesora files
        // - Helps with file recovery/forensics
        uint64_t magic = AesoraFileHeader::MAGIC;
        encrypted_file.insert(
            encrypted_file.end(),
            reinterpret_cast<unsigned char*>(&magic),
            reinterpret_cast<unsigned char*>(&magic) + 8
        );
        
        // Add salt (stored plaintext - salts don't need secrecy)
        encrypted_file.insert(encrypted_file.end(), salt.begin(), salt.end());
        
        // Add IV (stored plaintext - IVs don't need secrecy)
        encrypted_file.insert(encrypted_file.end(), iv.begin(), iv.end());
        
        // Add ciphertext + GCM tag
        encrypted_file.insert(encrypted_file.end(), ciphertext_with_tag.begin(), ciphertext_with_tag.end());
        
        // Step 7: Write Aesora file to disk
        write_file_binary(output_file, encrypted_file);
        
        // Optional: Print success message
        std::cout << "Encryption successful!\n"
                  << "Input file: " << input_file << "\n"
                  << "Output file: " << output_file << "\n"
                  << "Original size: " << plaintext.size() << " bytes\n"
                  << "Encrypted size: " << encrypted_file.size() << " bytes\n"
                  << "Overhead: " << (encrypted_file.size() - plaintext.size()) << " bytes\n";
        
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Encryption failed: ") + e.what());
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Encryption failed (unknown error): ") + e.what());
    }
}
