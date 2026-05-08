/**
 * @file file_utils.cpp
 * @brief Implementation of file I/O utilities
 * 
 * All operations use binary mode (no text conversions).
 * Supports any file type (text, binary, encrypted, etc.)
 */

#include "file_utils.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>
#include <iostream>

namespace fs = std::filesystem;

std::vector<unsigned char> read_file_binary(const std::string& path) {
    // SECURITY: Open in binary mode to prevent text conversions
    // TEXT MODE DANGER:
    // - On Windows, '\n' (LF) converted to '\r\n' (CRLF)
    // - On Unix, no conversion but encoding might be interpreted
    // - For encrypted data, ANY conversion corrupts bytes
    // - ALWAYS use binary mode for any non-text data
    
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for reading: " + path);
    }
    
    // ios::ate positions file pointer at end, giving us file size
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    if (file_size < 0) {
        throw std::runtime_error("Failed to determine file size: " + path);
    }
    
    std::vector<unsigned char> buffer(file_size);
    
    // Read entire file into memory
    // NOTE: For very large files, consider chunked reading to reduce memory usage
    if (!file.read(reinterpret_cast<char*>(buffer.data()), file_size)) {
        throw std::runtime_error("Failed to read file: " + path);
    }
    
    return buffer;
}

void write_file_binary(
    const std::string& path,
    const std::vector<unsigned char>& data,
    [[maybe_unused]] int mode
) {
    // SECURITY: Open in binary mode to prevent text conversions
    // TRUNCATE: If file exists, we overwrite it completely
    // REASON: Previous data might contain sensitive information
    
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + path);
    }
    
    // Write all data
    if (!file.write(reinterpret_cast<const char*>(data.data()), data.size())) {
        throw std::runtime_error("Failed to write file: " + path);
    }
    
    file.close();
    
    // SECURITY: Set file permissions on Unix-like systems
    // WHY: Encrypted files should be readable only by owner
    // DEFAULT UMASK: 0644 (owner: rw, others: r)
    // RECOMMENDATION: Use 0600 (owner: rw, others: ---) for encrypted files
    // WINDOWS NOTE: Ignores this, uses directory permissions instead
    
    #ifndef _WIN32
    try {
        fs::permissions(path, fs::perm_options::perm_format | mode);
    } catch (...) {
        // Non-fatal: file is written, just permissions might be wrong
        std::cerr << "Warning: Could not set file permissions for: " << path << std::endl;
    }
    #endif
}

bool file_exists(const std::string& path) {
    try {
        return fs::exists(path);
    } catch (...) {
        return false;
    }
}

std::size_t get_file_size(const std::string& path) {
    try {
        if (!fs::exists(path)) {
            return 0;
        }
        return fs::file_size(path);
    } catch (...) {
        return 0;
    }
}
