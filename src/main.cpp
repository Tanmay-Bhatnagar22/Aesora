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
#include <fstream>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
#endif
#include <file_utils.h>

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
 * Prints welcome banner and menu information.
 */
void print_banner() {
    std::cout << "\n";
    std::cout << "=============================================\n";
    std::cout << "     Aesora: Secure File Encryption Tool\n";
    std::cout << "=============================================\n\n";
    std::cout << "\n";
    std::cout << "SECURITY FEATURES:\n";
    std::cout << "  [+] AES-256-GCM for authenticated encryption\n";
    std::cout << "  [+] PBKDF2-HMAC-SHA256 with 310,000 iterations\n";
    std::cout << "  [+] Random salt and IV for each encryption\n";
    std::cout << "  [+] Authentication tag verifies file integrity\n";
    std::cout << "  [+] Password prompted securely (not via command line)\n";
    std::cout << "\n";
}

/**
 * Print success message.
 */
void print_success(const std::string& message) {
    std::cout << "[+] " << message << "\n";
}

/**
 * Print error message.
 */
void print_error(const std::string& message) {
    std::cerr << "[-] " << message << "\n";
}

/**
 * Print info message.
 */
void print_info(const std::string& message) {
    std::cout << "[*] " << message << "\n";
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    try {
        print_banner();
        
        // Ask user for operation type
        std::string operation;
        std::cout << "[*] Do you want to (E)ncrypt or (D)ecrypt? [E/D]: ";
        std::getline(std::cin, operation);
        
        // Convert to uppercase for case-insensitive comparison
        if (!operation.empty()) {
            operation[0] = std::toupper(operation[0]);
        }
        
        bool is_encrypt = (operation == "E");
        bool is_decrypt = (operation == "D");
        
        if (!is_encrypt && !is_decrypt) {
            print_error("Invalid option. Please enter 'E' or 'D'");
            return 1;
        }
        
        // Ask user for file path
        std::string prompt = is_encrypt ? 
            "[*] Enter the path to the file you want to encrypt:\n> " :
            "[*] Enter the path to the file you want to decrypt:\n> ";
        
        std::string input_file;
        std::cout << prompt;
        std::getline(std::cin, input_file);
        
        // Trim whitespace
        input_file.erase(0, input_file.find_first_not_of(" \t\n\r"));
        input_file.erase(input_file.find_last_not_of(" \t\n\r") + 1);
        
        // Validate file exists
        if (!file_exists(input_file)) {
            print_error("File not found: '" + input_file + "'");
            return 1;
        }
        
        // Prepare output file path
        std::string output_file;
        if (is_encrypt) {
            output_file = input_file + ".aesora";
        } else {
            // Remove .aesora extension if present, otherwise append .decrypted
            if (input_file.length() > 7 && input_file.substr(input_file.length() - 7) == ".aesora") {
                output_file = input_file.substr(0, input_file.length() - 7);
            } else {
                output_file = input_file + ".decrypted";
            }
        }
        
        print_info("File to " + std::string(is_encrypt ? "encrypt" : "decrypt") + ": " + input_file);
        print_info("Output file:     " + output_file);
        
        // Prompt for password securely
        std::string password_prompt = is_encrypt ? 
            "\n[*] Enter password for encryption: " :
            "\n[*] Enter password for decryption: ";
        
        std::string password = get_password_from_user(password_prompt);
        
        if (is_encrypt) {
            print_info("Encrypting file...");
            encrypt_file(input_file, output_file, password);
            print_success("File encrypted successfully!");
        } else {
            print_info("Decrypting file...");
            decrypt_file(input_file, output_file, password);
            print_success("File decrypted successfully!");
        }
        
        print_success("Output file saved to: " + output_file);
        
        // Clear password from memory
        for (size_t i = 0; i < password.size(); ++i) {
            password[i] = '\0';
        }
        password.clear();
        
        return 0;
        
    } catch (const std::runtime_error& e) {
        print_error(std::string(e.what()));
        return 1;
    } catch (const std::exception& e) {
        print_error(std::string("UNEXPECTED: ") + e.what());
        return 1;
    }
}
