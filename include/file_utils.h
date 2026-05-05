/**
 * @file file_utils.h
 * @brief File I/O utilities for Aesora
 * 
 * This module handles binary file operations safely:
 * - Reading files in chunks to minimize memory usage
 * - Writing encrypted data securely
 * - File validation and error handling
 * 
 * DESIGN PRINCIPLES:
 * - Binary-safe: Works with any file type (text, images, archives, etc.)
 * - Memory-efficient: Reads large files in chunks
 * - Error-transparent: All operations throw exceptions on failure
 */

#ifndef FILE_UTILS_H
#define FILE_UTILS_H
#include <cstdint>
#include <vector>
#include <string>
#include <cstddef>

/**
 * Reads entire file into memory as binary data.
 * 
 * PURPOSE:
 * Loads a file for encryption. For large files, consider chunked reading
 * to avoid excessive memory usage.
 * 
 * PARAMETERS:
 * - path: Filesystem path to the file (supports relative and absolute paths)
 * 
 * SECURITY CONSIDERATIONS:
 * - File is read in binary mode (no text conversion)
 * - Returns entire file in memory (may use significant RAM for large files)
 * - File data is not cleared after return; caller is responsible
 * - Symlinks are followed (follow_symlinks can be modified if needed)
 * 
 * @param path  Path to file to read
 * @return      Binary contents of file
 * @throws      std::runtime_error if file cannot be opened or read
 */
std::vector<unsigned char> read_file_binary(const std::string& path);

/**
 * Writes binary data to a file.
 * 
 * PURPOSE:
 * Safely writes encrypted data or decrypted plaintext to disk.
 * Creates parent directories if they don't exist (can be disabled).
 * 
 * PARAMETERS:
 * - path:  Output file path
 * - data:  Binary data to write
 * - mode:  File creation mode (0644 by default on Unix, ignored on Windows)
 * 
 * SECURITY CONSIDERATIONS:
 * - File is opened in binary mode (no encoding conversion)
 * - File is truncated if it already exists (overwrites previous content)
 * - On Unix, default permissions are 0644 (readable by all, writable by owner)
 * - For encrypted files, consider using 0600 (readable/writable by owner only)
 * - Windows ignores file mode parameter (security inherited from directory)
 * - Data is NOT securely wiped from memory after write; use clear_sensitive_data if needed
 * 
 * @param path  Path where file will be written
 * @param data  Binary data to write to file
 * @param mode  File permissions (Unix-like systems only, default 0644)
 * @throws      std::runtime_error if write fails
 */
void write_file_binary(
    const std::string& path,
    const std::vector<unsigned char>& data,
    int mode = 0644
);

/**
 * Checks if a file exists at the given path.
 * 
 * PURPOSE:
 * Verifies file existence before attempting to read/process.
 * Useful for validating input file before encryption/decryption.
 * 
 * SECURITY CONSIDERATIONS:
 * - TOCTOU race condition possible if file deleted between check and use
 * - For security-critical operations, let open() fail naturally
 * - Does not check if path is readable by current process
 * 
 * @param path  Path to check
 * @return      true if file exists and is readable, false otherwise
 */
bool file_exists(const std::string& path);

/**
 * Gets file size in bytes.
 * 
 * PURPOSE:
 * Determines file size for progress indication or pre-allocation.
 * Useful for validating that encrypted files have minimum size (header + data).
 * 
 * SECURITY CONSIDERATIONS:
 * - Returns 0 for non-existent files (not distinguishable from empty files)
 * - File size doesn't guarantee file integrity
 * - For integrity verification, rely on GCM authentication tag
 * 
 * @param path  Path to file
 * @return      File size in bytes, or 0 if file doesn't exist
 */
std::size_t get_file_size(const std::string& path);

/**
 * Structure representing an Aesora encrypted file.
 * 
 * FILE FORMAT:
 * [8-byte Magic Header] [16-byte Salt] [12-byte IV] [Ciphertext...] [16-byte GCM Tag]
 * 
 * Total minimum size: 8 + 16 + 12 + 0 + 16 = 52 bytes (empty file)
 * 
 * Magic header: 0x41455355524100 (ASCII: "AESURA\0" in hex)
 * - Identifies file as Aesora encrypted file
 * - Prevents accidental decryption of non-Aesora files
 */
struct AesoraFileHeader {
    static constexpr uint64_t MAGIC = 0x4145534F52410000ULL;  // "AESORA\0\0" magic number
    static constexpr size_t SALT_SIZE = 16;      // PBKDF2 salt size (bytes)
    static constexpr size_t IV_SIZE = 12;        // AES-GCM IV/nonce size (bytes)
    static constexpr size_t GCM_TAG_SIZE = 16;   // GCM authentication tag size (bytes)
    static constexpr size_t HEADER_SIZE = 8 + SALT_SIZE + IV_SIZE;  // Metadata before ciphertext
};

#endif // FILE_UTILS_H
