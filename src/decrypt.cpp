/**
 * @file decrypt.cpp
 * @brief Implementation of file decryption
 * 
 * This module orchestrates:
 * 1. Aesora file format parsing
 * 2. Key derivation from password
 * 3. AES-256-GCM decryption with authentication
 */

#include "decrypt.h"
#include "crypto_utils.h"
#include "file_utils.h"

#include <cstring>
#include <stdexcept>
#include <iostream>

void decrypt_file(
    const std::string& input_file,
    const std::string& output_file,
    const std::string& password
) {
    try {
        // Step 1: Read encrypted file
        // WHY BINARY READ:
        // - Aesora files are binary (contain arbitrary byte sequences)
        // - Any text conversion corrupts the encrypted data
        std::vector<unsigned char> encrypted_file = read_file_binary(input_file);
        
        // Step 2: Validate file format and size
        // MINIMUM FILE SIZE:
        // = 8 (magic) + 16 (salt) + 12 (IV) + 0 (ciphertext) + 16 (tag)
        // = 52 bytes
        // 
        // For 1 byte plaintext:
        // = 8 + 16 + 12 + 1 + 16 = 53 bytes
        if (encrypted_file.size() < 52) {
            throw std::runtime_error(
                "Encrypted file too small (minimum 52 bytes). "
                "File may be corrupted or not an Aesora file."
            );
        }
        
        // Step 3: Extract and verify magic header
        // MAGIC NUMBER: 0x4145534F52410000 (ASCII: "AESORA\0\0")
        // PURPOSE: Identifies Aesora files, prevents decrypting non-Aesora files
        uint64_t file_magic = 0;
        std::memcpy(&file_magic, encrypted_file.data(), 8);
        
        if (file_magic != AesoraFileHeader::MAGIC) {
            throw std::runtime_error(
                "Invalid magic header. File is not an Aesora encrypted file. "
                "Magic: 0x" + std::to_string(file_magic)
            );
        }
        
        // Step 4: Extract salt, IV, and ciphertext
        // FILE LAYOUT:
        // [0-7]:     Magic header (8 bytes)
        // [8-23]:    Salt (16 bytes)
        // [24-35]:   IV (12 bytes)
        // [36+]:     Ciphertext + GCM tag (remaining bytes)
        
        size_t offset = 0;
        offset += 8;  // Skip magic
        
        std::vector<unsigned char> salt(
            encrypted_file.begin() + offset,
            encrypted_file.begin() + offset + AesoraFileHeader::SALT_SIZE
        );
        offset += AesoraFileHeader::SALT_SIZE;
        
        std::vector<unsigned char> iv(
            encrypted_file.begin() + offset,
            encrypted_file.begin() + offset + AesoraFileHeader::IV_SIZE
        );
        offset += AesoraFileHeader::IV_SIZE;
        
        // Remaining bytes are ciphertext + GCM tag
        // GCM tag is 16 bytes at the end
        std::vector<unsigned char> ciphertext_with_tag(
            encrypted_file.begin() + offset,
            encrypted_file.end()
        );
        
        // VALIDATION: Ensure at least 16 bytes for GCM tag
        if (ciphertext_with_tag.size() < 16) {
            throw std::runtime_error(
                "Ciphertext too short (missing GCM authentication tag). File may be corrupted."
            );
        }
        
        // Step 5: Derive key from password (same PBKDF2 parameters as encryption)
        // WHY SAME PARAMETERS:
        // - To get same key as encryption, must use same:
        //   a) Password (user-provided)
        //   b) Salt (extracted from file)
        //   c) Iterations (310,000 - must match)
        // - If any differs, decryption key will be wrong
        // - Wrong key -> GCM tag verification fails
        std::vector<unsigned char> key = derive_key_from_password(password, salt);
        
        // Step 6: Decrypt and verify authentication tag
        // WHY VERIFY TAG:
        // - GCM tag proves:
        //   a) Data integrity (not corrupted)
        //   b) Authenticity (not modified by attacker)
        //   c) Correct password (wrong password = wrong key = wrong tag)
        // - If tag fails, we return error without revealing plaintext
        // - Attacker cannot distinguish between wrong password and tampering
        
        std::vector<unsigned char> plaintext = decrypt_aes256_gcm(ciphertext_with_tag, key, iv);
        
        // Clear key from memory (very important)
        // WHY:
        // - After decryption, key is no longer needed
        // - Key in memory = potential compromise if attacker has RAM access
        clear_sensitive_data(key);
        
        // Step 7: Write decrypted plaintext to output file
        // WHY BINARY WRITE:
        // - Preserves original file exactly (no encoding conversion)
        // - Works with any file type
        write_file_binary(output_file, plaintext);
        
        // Optional: Clear plaintext from memory
        // WHY CLEAR:
        // - After writing to disk, plaintext in RAM is unnecessary
        // - Reduces exposure time if attacker has RAM access
        // - Defense-in-depth strategy
        clear_sensitive_data(plaintext);
        
        // Step 8: Success message
        std::cout << "Decryption successful!\n"
                  << "Encrypted file: " << input_file << "\n"
                  << "Decrypted file: " << output_file << "\n"
                  << "Plaintext size: " << plaintext.size() << " bytes\n"
                  << "Encryption overhead: " << (encrypted_file.size() - plaintext.size()) << " bytes\n";
        
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Decryption failed: ") + e.what());
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Decryption failed (unknown error): ") + e.what());
    }
}
