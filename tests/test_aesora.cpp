/**
 * @file test_aesora.cpp
 * @brief Test suite for Aesora encryption tool
 * 
 * TESTS COVER:
 * - Basic encryption/decryption
 * - Wrong password detection
 * - File corruption detection
 * - Binary file support
 * - Edge cases
 */

#include "encrypt.h"
#include "decrypt.h"
#include "crypto_utils.h"
#include "file_utils.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

// Test counter
int tests_passed = 0;
int tests_failed = 0;

/**
 * Test helper: Assert with description
 */
void assert_test(bool condition, const std::string& test_name) {
    if (condition) {
        std::cout << "✓ PASS: " << test_name << "\n";
        tests_passed++;
    } else {
        std::cout << "✗ FAIL: " << test_name << "\n";
        tests_failed++;
    }
}

/**
 * Test helper: Create temporary test file
 */
std::string create_test_file(const std::string& content) {
    static int counter = 0;
    std::string filename = "test_file_" + std::to_string(counter++) + ".txt";
    
    std::ofstream file(filename, std::ios::binary);
    file.write(content.c_str(), content.size());
    file.close();
    
    return filename;
}

/**
 * Test helper: Read file contents
 */
std::string read_test_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/**
 * Test 1: Basic encryption and decryption
 */
void test_basic_encryption_decryption() {
    std::cout << "\n=== Test 1: Basic Encryption/Decryption ===\n";
    
    std::string plaintext = "Hello, Aesora! This is a test message.";
    std::string input_file = create_test_file(plaintext);
    std::string encrypted_file = "test_encrypted_1.aesora";
    std::string decrypted_file = "test_decrypted_1.txt";
    std::string password = "TestPassword123!";
    
    try {
        // Encrypt
        encrypt_file(input_file, encrypted_file, password);
        assert_test(file_exists(encrypted_file), "Encrypted file created");
        
        // Encrypted file should be larger (overhead)
        size_t plaintext_size = plaintext.size();
        size_t encrypted_size = get_file_size(encrypted_file);
        assert_test(encrypted_size > plaintext_size, "Encrypted file larger than plaintext");
        
        // Decrypt
        decrypt_file(encrypted_file, decrypted_file, password);
        assert_test(file_exists(decrypted_file), "Decrypted file created");
        
        // Verify content matches
        std::string recovered = read_test_file(decrypted_file);
        assert_test(recovered == plaintext, "Decrypted content matches original");
        
        // Cleanup
        fs::remove(input_file);
        fs::remove(encrypted_file);
        fs::remove(decrypted_file);
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        assert_test(false, "Basic encryption/decryption");
    }
}

/**
 * Test 2: Wrong password detection
 */
void test_wrong_password() {
    std::cout << "\n=== Test 2: Wrong Password Detection ===\n";
    
    std::string plaintext = "Secret message";
    std::string input_file = create_test_file(plaintext);
    std::string encrypted_file = "test_encrypted_2.aesora";
    std::string decrypted_file = "test_decrypted_2.txt";
    std::string correct_password = "CorrectPassword";
    std::string wrong_password = "WrongPassword";
    
    try {
        // Encrypt with correct password
        encrypt_file(input_file, encrypted_file, correct_password);
        
        // Try to decrypt with wrong password - should fail
        bool decryption_failed = false;
        try {
            decrypt_file(encrypted_file, decrypted_file, wrong_password);
        } catch (const std::runtime_error& e) {
            // Expected to fail
            std::string error_msg = e.what();
            if (error_msg.find("Authentication") != std::string::npos) {
                decryption_failed = true;
            }
        }
        
        assert_test(decryption_failed, "Wrong password detected (authentication failed)");
        
        // Cleanup
        fs::remove(input_file);
        fs::remove(encrypted_file);
        if (file_exists(decrypted_file)) {
            fs::remove(decrypted_file);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        assert_test(false, "Wrong password detection");
    }
}

/**
 * Test 3: Binary file support
 */
void test_binary_file() {
    std::cout << "\n=== Test 3: Binary File Support ===\n";
    
    // Create a binary file with arbitrary bytes
    std::string input_file = "test_binary.bin";
    unsigned char binary_data[] = {
        0x00, 0x01, 0x02, 0xFF, 0xFE, 0xFD,
        0x89, 0x50, 0x4E, 0x47,  // PNG magic bytes
        0x00, 0x00, 0xFF, 0xFF
    };
    
    std::ofstream out(input_file, std::ios::binary);
    out.write(reinterpret_cast<char*>(binary_data), sizeof(binary_data));
    out.close();
    
    std::string encrypted_file = "test_encrypted_3.aesora";
    std::string decrypted_file = "test_decrypted_3.bin";
    std::string password = "BinaryTest";
    
    try {
        // Encrypt binary file
        encrypt_file(input_file, encrypted_file, password);
        
        // Decrypt
        decrypt_file(encrypted_file, decrypted_file, password);
        
        // Verify binary content matches exactly
        std::vector<unsigned char> original = read_file_binary(input_file);
        std::vector<unsigned char> recovered = read_file_binary(decrypted_file);
        assert_test(original == recovered, "Binary file content matches exactly");
        
        // Cleanup
        fs::remove(input_file);
        fs::remove(encrypted_file);
        fs::remove(decrypted_file);
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        assert_test(false, "Binary file support");
    }
}

/**
 * Test 4: Empty file encryption
 */
void test_empty_file() {
    std::cout << "\n=== Test 4: Empty File Encryption ===\n";
    
    std::string input_file = create_test_file("");  // Empty file
    std::string encrypted_file = "test_encrypted_4.aesora";
    std::string decrypted_file = "test_decrypted_4.txt";
    std::string password = "EmptyFileTest";
    
    try {
        // Encrypt empty file
        encrypt_file(input_file, encrypted_file, password);
        
        // Decrypt
        decrypt_file(encrypted_file, decrypted_file, password);
        
        // Verify it's still empty
        std::string recovered = read_test_file(decrypted_file);
        assert_test(recovered.empty(), "Empty file remains empty");
        
        // Cleanup
        fs::remove(input_file);
        fs::remove(encrypted_file);
        fs::remove(decrypted_file);
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        assert_test(false, "Empty file encryption");
    }
}

/**
 * Test 5: Large file handling
 */
void test_large_file() {
    std::cout << "\n=== Test 5: Large File Handling ===\n";
    
    // Create a 10MB file
    std::string input_file = "test_large.bin";
    std::ofstream out(input_file, std::ios::binary);
    
    const size_t chunk_size = 1024 * 1024;  // 1MB chunks
    const int num_chunks = 10;              // 10MB total
    
    std::vector<unsigned char> chunk(chunk_size, 0xAB);
    for (int i = 0; i < num_chunks; ++i) {
        out.write(reinterpret_cast<char*>(chunk.data()), chunk_size);
    }
    out.close();
    
    std::string encrypted_file = "test_encrypted_5.aesora";
    std::string decrypted_file = "test_decrypted_5.bin";
    std::string password = "LargeFileTest";
    
    try {
        // Encrypt
        encrypt_file(input_file, encrypted_file, password);
        assert_test(get_file_size(encrypted_file) > 10 * 1024 * 1024, "Encrypted file larger than input");
        
        // Decrypt
        decrypt_file(encrypted_file, decrypted_file, password);
        
        // Verify size
        size_t original_size = get_file_size(input_file);
        size_t recovered_size = get_file_size(decrypted_file);
        assert_test(original_size == recovered_size, "Large file size preserved");
        
        // Cleanup
        fs::remove(input_file);
        fs::remove(encrypted_file);
        fs::remove(decrypted_file);
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        assert_test(false, "Large file handling");
    }
}

/**
 * Test 6: Corrupted encrypted file detection
 */
void test_corrupted_file() {
    std::cout << "\n=== Test 6: Corrupted File Detection ===\n";
    
    std::string plaintext = "Secret data";
    std::string input_file = create_test_file(plaintext);
    std::string encrypted_file = "test_encrypted_6.aesora";
    std::string decrypted_file = "test_decrypted_6.txt";
    std::string password = "CorruptionTest";
    
    try {
        // Encrypt
        encrypt_file(input_file, encrypted_file, password);
        
        // Corrupt the encrypted file (flip a byte in the middle)
        std::vector<unsigned char> corrupted = read_file_binary(encrypted_file);
        if (corrupted.size() > 50) {
            corrupted[50] ^= 0xFF;  // Flip all bits in middle byte
            write_file_binary(encrypted_file, corrupted);
        }
        
        // Try to decrypt corrupted file - should fail
        bool decryption_failed = false;
        try {
            decrypt_file(encrypted_file, decrypted_file, password);
        } catch (const std::runtime_error& e) {
            // Expected to fail
            std::string error_msg = e.what();
            if (error_msg.find("Authentication") != std::string::npos) {
                decryption_failed = true;
            }
        }
        
        assert_test(decryption_failed, "Corrupted file detected (authentication failed)");
        
        // Cleanup
        fs::remove(input_file);
        fs::remove(encrypted_file);
        if (file_exists(decrypted_file)) {
            fs::remove(decrypted_file);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        assert_test(false, "Corrupted file detection");
    }
}

/**
 * Test 7: Special characters in password
 */
void test_special_password() {
    std::cout << "\n=== Test 7: Special Characters in Password ===\n";
    
    std::string plaintext = "Test message";
    std::string input_file = create_test_file(plaintext);
    std::string encrypted_file = "test_encrypted_7.aesora";
    std::string decrypted_file = "test_decrypted_7.txt";
    
    // Password with special characters
    std::string password = "P@$$w0rd!#%&*()[]{}";
    
    try {
        // Encrypt
        encrypt_file(input_file, encrypted_file, password);
        
        // Decrypt with same special password
        decrypt_file(encrypted_file, decrypted_file, password);
        
        // Verify content
        std::string recovered = read_test_file(decrypted_file);
        assert_test(recovered == plaintext, "Special characters in password work");
        
        // Cleanup
        fs::remove(input_file);
        fs::remove(encrypted_file);
        fs::remove(decrypted_file);
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        assert_test(false, "Special characters in password");
    }
}

/**
 * Test 8: Crypto utilities - key derivation consistency
 */
void test_key_derivation_consistency() {
    std::cout << "\n=== Test 8: Key Derivation Consistency ===\n";
    
    try {
        std::string password = "TestPassword";
        std::vector<unsigned char> salt = generate_random_bytes(16);
        
        // Derive key twice with same parameters
        auto key1 = derive_key_from_password(password, salt, 310000);
        auto key2 = derive_key_from_password(password, salt, 310000);
        
        // Keys should be identical
        assert_test(key1 == key2, "Same password/salt produces same key");
        
        // Different salt should produce different key
        auto salt2 = generate_random_bytes(16);
        auto key3 = derive_key_from_password(password, salt2, 310000);
        assert_test(key1 != key3, "Different salt produces different key");
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        assert_test(false, "Key derivation consistency");
    }
}

/**
 * Main test runner
 */
int main() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "AESORA TEST SUITE\n";
    std::cout << std::string(60, '=') << "\n";
    
    // Run all tests
    test_basic_encryption_decryption();
    test_wrong_password();
    test_binary_file();
    test_empty_file();
    test_large_file();
    test_corrupted_file();
    test_special_password();
    test_key_derivation_consistency();
    
    // Summary
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "TEST SUMMARY\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << "Tests Passed: " << tests_passed << "\n";
    std::cout << "Tests Failed: " << tests_failed << "\n";
    std::cout << "Total Tests:  " << (tests_passed + tests_failed) << "\n";
    std::cout << std::string(60, '=') << "\n\n";
    
    return tests_failed == 0 ? 0 : 1;
}
