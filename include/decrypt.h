/**
 * @file decrypt.h
 * @brief File decryption interface for Aesora
 * 
 * This module provides the main decryption API.
 * Orchestrates:
 * - Aesora file format parsing
 * - Key derivation from password
 * - AES-256-GCM decryption with authentication verification
 */

#ifndef DECRYPT_H
#define DECRYPT_H

#include <string>

/**
 * Decrypts an Aesora-encrypted file with a user password.
 * 
 * PURPOSE:
 * Main decryption operation. Recovers the original file from an Aesora
 * encrypted file using the same password that was used for encryption.
 * Cryptographically verifies authenticity using AES-256-GCM authentication tag.
 * 
 * PARAMETERS:
 * - input_file:   Path to Aesora encrypted file
 * - output_file:  Path where plaintext will be written
 * - password:     User's password (must match encryption password exactly)
 * 
 * DECRYPTION FLOW:
 * 1. Read Aesora file and verify magic header
 * 2. Extract salt, IV, ciphertext, and GCM authentication tag
 * 3. Derive key from password + salt using same PBKDF2 parameters as encryption
 * 4. Decrypt ciphertext using AES-256-GCM
 * 5. Verify GCM authentication tag
 * 6. If verification succeeds: write plaintext to output
 * 7. If verification fails: abort and signal tampering/wrong password
 * 
 * AUTHENTICATION VERIFICATION:
 * AES-256-GCM provides authenticated encryption with built-in authentication tag.
 * This tag is cryptographically verified during decryption:
 * 
 * - Correct password + unmodified ciphertext:
 *   Tag verifies successfully, plaintext returned
 * 
 * - Wrong password:
 *   Different key derived, tag verification fails, no plaintext returned
 *   (Attacker cannot distinguish wrong password from tampering)
 * 
 * - Corrupted ciphertext (accidental or malicious):
 *   Tag verification fails, no plaintext returned
 *   (Guarantees data integrity)
 * 
 * SECURITY CONSIDERATIONS:
 * - ALWAYS verifies GCM authentication tag (cryptographically authenticated decryption)
 * - If authentication fails, returns empty plaintext (no information leakage)
 * - Same PBKDF2 parameters used as encryption (310,000 iterations)
 * - File format validation prevents processing non-Aesora files
 * - Wrong password is indistinguishable from corrupted file (intentional for security)
 * - Password used directly (not hashed or stored), cleared after key derivation
 * 
 * ERROR HANDLING:
 * - Throws std::runtime_error on:
 *   a) File doesn't exist or cannot be read
 *   b) File is too small (corrupted/incomplete)
 *   c) Magic header doesn't match (not an Aesora file)
 *   d) File is corrupted (size mismatch)
 *   e) Cryptographic operation fails
 *   f) Output file cannot be written
 * 
 * - Special case: Authentication tag verification failure
 *   Throws std::runtime_error with message: "Authentication tag verification failed"
 *   This indicates either wrong password or corrupted file
 * 
 * USER EXPERIENCE:
 * Regardless of failure reason, user sees consistent error message to prevent
 * attackers from gaining information about whether password was "close" or file
 * was "partially correct".
 * 
 * MEMORY CONSIDERATIONS:
 * - Loads entire encrypted file into memory
 * - Allocates plaintext buffer during decryption
 * - For very large files (>available RAM), see chunked_decrypt_file
 * - Sensitive data (password, key, plaintext) cleared after use
 * 
 * PERFORMANCE:
 * - Key derivation: ~300-500ms (same PBKDF2 iteration cost)
 * - Decryption: Depends on file size (~100-200 MB/sec with AES-NI)
 * - Total time dominated by PBKDF2 key derivation
 * 
 * @param input_file   Path to Aesora encrypted file
 * @param output_file  Path where decrypted plaintext will be written
 * @param password     User's password (must match encryption password)
 * @throws             std::runtime_error with descriptive message on failure
 *                     - "Authentication tag verification failed" means wrong password or tampering
 *                     - Other errors describe file format/I/O issues
 * 
 * USAGE EXAMPLE:
 * ===============
 * try {
 *     decrypt_file("document.pdf.aesora", "document.pdf", "MyPassword123");
 *     std::cout << "Decryption successful!" << std::endl;
 * } catch (const std::runtime_error& e) {
 *     std::string error_msg = e.what();
 *     if (error_msg.find("Authentication") != std::string::npos) {
 *         std::cerr << "Wrong password or file was tampered with!" << std::endl;
 *     } else {
 *         std::cerr << "Decryption failed: " << error_msg << std::endl;
 *     }
 * }
 */
void decrypt_file(
    const std::string& input_file,
    const std::string& output_file,
    const std::string& password
);

#endif // DECRYPT_H
