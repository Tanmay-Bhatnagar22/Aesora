/**
 * @file encrypt.h
 * @brief File encryption interface for Aesora
 * 
 * This module provides the main encryption API.
 * Orchestrates:
 * - Random salt and IV generation
 * - Key derivation from password
 * - AES-256-GCM encryption
 * - Aesora file format construction
 */

#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <string>

/**
 * Encrypts a file with a user password.
 * 
 * PURPOSE:
 * Main encryption operation. Encrypts an arbitrary file and produces
 * an Aesora-format encrypted file that can be decrypted with the same password.
 * 
 * PARAMETERS:
 * - input_file:   Path to plaintext file (can be any file type)
 * - output_file:  Path to encrypted file (will be created/overwritten)
 * - password:     User's password (any string, including empty string)
 * 
 * SECURITY FLOW:
 * 1. Generate random 16-byte salt
 * 2. Generate random 12-byte IV
 * 3. Derive 32-byte key from password + salt using PBKDF2-HMAC-SHA256 (310,000 iterations)
 * 4. Encrypt plaintext using AES-256-GCM with key + IV
 * 5. Write Aesora file format:
 *    [Magic Header (8 bytes)] [Salt (16 bytes)] [IV (12 bytes)] [Ciphertext] [GCM Tag (16 bytes)]
 * 
 * FILE FORMAT RATIONALE:
 * - Magic header: Identifies Aesora files, prevents accidental decryption of non-Aesora data
 * - Salt stored plaintext: Required to reproduce key from password (salts don't need secrecy)
 * - IV stored plaintext: Required to decrypt (IVs don't need secrecy, only randomness)
 * - GCM Tag at end: Allows streaming verification, marks end of encrypted data
 * 
 * SECURITY CONSIDERATIONS:
 * - Random salt prevents rainbow tables and pre-computation attacks
 * - High iteration count (310,000) makes brute-force prohibitively expensive
 * - Random IV prevents patterns if same file encrypted multiple times
 * - AES-256-GCM provides both confidentiality and authenticity
 * - Output file will be larger than input (52 bytes minimum overhead)
 * 
 * ERROR HANDLING:
 * - Throws std::runtime_error on:
 *   a) Input file cannot be read
 *   b) Cryptographic operations fail
 *   c) Output file cannot be written
 * - Provides descriptive error messages for debugging
 * 
 * MEMORY CONSIDERATIONS:
 * - Loads entire input file into memory
 * - For very large files (>available RAM), see chunked_encrypt_file
 * - Sensitive data (password, key, plaintext) should be cleared after use
 * 
 * PERFORMANCE:
 * - Key derivation: ~300-500ms (PBKDF2 iteration cost)
 * - Encryption: Depends on file size (~100-200 MB/sec for AES-NI)
 * - Total time for typical file: mostly PBKDF2 time, followed by encryption
 * 
 * @param input_file   Path to file to encrypt
 * @param output_file  Path to encrypted output file
 * @param password     User's password (any length, any characters)
 * @throws             std::runtime_error with descriptive message on failure
 * 
 * USAGE EXAMPLE:
 * ===============
 * try {
 *     encrypt_file("document.pdf", "document.pdf.aesora", "MyPassword123");
 *     std::cout << "Encryption successful!" << std::endl;
 * } catch (const std::runtime_error& e) {
 *     std::cerr << "Encryption failed: " << e.what() << std::endl;
 * }
 */
void encrypt_file(
    const std::string& input_file,
    const std::string& output_file,
    const std::string& password
);

#endif // ENCRYPT_H
