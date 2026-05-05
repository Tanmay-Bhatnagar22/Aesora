/**
 * @file main.cpp
 * @brief CLI interface for Aesora
 * 
 * USAGE:
 * ======
 * aesora encrypt <input_file> <output_file>
 * aesora decrypt <input_file> <output_file>
 * 
 * PASSWORD HANDLING:
 * ==================
 * - Password is prompted interactively from terminal
 * - NOT passed via command line (avoids process list exposure)
 * - WHY: Any argument on command line is visible in:
 *   a) Process listings (ps, top, tasklist)
 *   b) Shell history
 *   c) System audit logs
 *   d) Core dumps
 * - Interactive prompt is not visible externally
 * 
 * SECURITY:
 * =========
 * - Uses getpass() on Unix, Windows API on Windows
 * - Disables echo so password is not visible while typing
 * - Clears password from memory after use
 */

#include "encrypt.h"
#include "decrypt.h"
#include "crypto_utils.h"

#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
#endif

/**
 * Securely prompts user for password without echoing input.
 * 
 * SECURITY:
 * - Input is not shown on screen (prevents shoulder surfing)
 * - Works on Windows and Unix-like systems
 * 
 * @param prompt  Message to display to user
 * @return        User's password
 */
std::string get_password_from_user(const std::string& prompt) {
    std::cout << prompt;
    std::cout.flush();
    
    std::string password;
    
    #ifdef _WIN32
    {
        // Windows: Use GetConsoleMode / SetConsoleMode to hide input
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        DWORD mode = 0;
        GetConsoleMode(hStdin, &mode);
        SetConsoleMode(hStdin, mode & ~ENABLE_ECHO_INPUT);
        
        char c;
        while (std::cin.get(c) && c != '\n') {
            password += c;
        }
        
        SetConsoleMode(hStdin, mode);
        std::cout << "\n";  // New line after password entry
    }
    #else
    {
        // Unix: Use termios to disable echo
        termios old_settings, new_settings;
        tcgetattr(STDIN_FILENO, &old_settings);
        new_settings = old_settings;
        new_settings.c_lflag &= ~ECHO;  // Disable echo
        tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
        
        char c;
        while (std::cin.get(c) && c != '\n') {
            password += c;
        }
        
        tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
        std::cout << "\n";  // New line after password entry
    }
    #endif
    
    return password;
}

/**
 * Prints usage information to the user.
 */
void print_usage() {
    std::cout << "\n"
              << "=== Aesora: Secure File Encryption Tool ===\n"
              << "\n"
              << "USAGE:\n"
              << "------\n"
              << "  aesora encrypt <input_file> <output_file>\n"
              << "  aesora decrypt <input_file> <output_file>\n"
              << "\n"
              << "EXAMPLES:\n"
              << "---------\n"
              << "  Encrypt a file:\n"
              << "    aesora encrypt document.pdf document.pdf.aesora\n"
              << "\n"
              << "  Decrypt a file:\n"
              << "    aesora decrypt document.pdf.aesora document.pdf\n"
              << "\n"
              << "SECURITY:\n"
              << "---------\n"
              << "  - Password is prompted interactively (not via command line)\n"
              << "  - Uses AES-256-GCM for authenticated encryption\n"
              << "  - Uses PBKDF2-HMAC-SHA256 with 310,000 iterations for key derivation\n"
              << "  - Random salt and IV for each encryption\n"
              << "  - Authentication tag verifies file integrity\n"
              << "\n";
}

int main(int argc, char* argv[]) {
    try {
        // Parse command line arguments
        if (argc < 4) {
            print_usage();
            return 1;
        }
        
        std::string command = argv[1];
        std::string input_file = argv[2];
        std::string output_file = argv[3];
        
        // Validate command
        if (command != "encrypt" && command != "decrypt") {
            std::cerr << "Error: Unknown command '" << command << "'\n";
            std::cerr << "Valid commands: encrypt, decrypt\n\n";
            print_usage();
            return 1;
        }
        
        // Prompt for password securely
        // WHY INTERACTIVE:
        // - Never visible in process list
        // - Never stored in shell history
        // - Securely cleared from memory after use
        std::string password = get_password_from_user(
            command == "encrypt" 
                ? "Enter password for encryption: " 
                : "Enter password for decryption: "
        );
        
        // Perform requested operation
        if (command == "encrypt") {
            // ENCRYPTION FLOW:
            // 1. Generate random salt (16 bytes)
            // 2. Generate random IV (12 bytes)
            // 3. Derive key from password using PBKDF2 (~300ms)
            // 4. Encrypt with AES-256-GCM
            // 5. Write Aesora file format
            encrypt_file(input_file, output_file, password);
        } else {
            // DECRYPTION FLOW:
            // 1. Read Aesora file
            // 2. Extract salt and IV from file
            // 3. Derive key from password using same salt/iterations
            // 4. Decrypt with AES-256-GCM
            // 5. Verify GCM authentication tag
            // 6. Write plaintext
            decrypt_file(input_file, output_file, password);
        }
        
        // Clear password from memory
        // WHY IMPORTANT:
        // - Password was used for key derivation
        // - Should not remain in memory
        // - Prevents recovery from RAM dumps
        for (size_t i = 0; i < password.size(); ++i) {
            password[i] = '\0';
        }
        password.clear();
        
        return 0;
        
    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "UNEXPECTED ERROR: " << e.what() << "\n";
        return 1;
    }
}
