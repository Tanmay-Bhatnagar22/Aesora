/**
 * @file main.cpp
 * @brief CLI interface for Aesora - Secure File Encryption Tool
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
 * 
 * UI/UX:
 * ======
 * - Professional ANSI color styling
 * - Windows ANSI compatibility for modern terminals
 * - Cybersecurity-oriented visual design
 */

#include "encrypt.h"
#include "decrypt.h"
#include "crypto_utils.h"
#include "terminal_colors.h"

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
    print_prompt(prompt);
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
 * Prints professional colored banner for Aesora.
 * Features: Bold cyan header, security feature list, visual separators
 */
void print_banner() {
    std::cout << "\n";
    print_banner_line("==================================================");
    print_banner_line("     " COLOR_BOLD_CYAN "Aesora" COLOR_RESET " - Secure File Encryption      ");
    print_banner_line("     Professional Cybersecurity Tool     ");
    print_banner_line("==================================================");
    
    print_section_header("SECURITY FEATURES");
    
    print_feature(FEATURE_AES " for authenticated encryption");
    print_feature(FEATURE_PBKDF2 " with 310,000 iterations");
    print_feature(FEATURE_SALT " for each encryption");
    print_feature(FEATURE_IV " (96-bit) for uniqueness");
    print_feature(FEATURE_GCM_TAG " verifies file integrity");
    print_feature("Password prompted securely (not via command line)");
    
    std::cout << "\n";
    print_separator();
    std::cout << "\n";
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    try {
        // Enable ANSI color support on Windows terminals
        enable_ansi_support();
        
        print_banner();
        
        // Ask user for operation type
        print_prompt("Select operation: " COLOR_BOLD_GREEN "(E)" COLOR_RESET "ncrypt or " COLOR_BOLD_RED "(D)" COLOR_RESET "ecrypt? [E/D]: ");
        
        std::string operation;
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
        
        // Print operation header
        if (is_encrypt) {
            print_section_header(COLOR_BOLD_GREEN "ENCRYPTION MODE" COLOR_RESET);
        } else {
            print_section_header(COLOR_BOLD_CYAN "DECRYPTION MODE" COLOR_RESET);
        }
        
        // Ask user for file path
        std::string file_prompt = is_encrypt ? 
            "\nEnter path to file to encrypt:\n" COLOR_BRIGHT_CYAN "> " COLOR_RESET :
            "\nEnter path to file to decrypt:\n" COLOR_BRIGHT_CYAN "> " COLOR_RESET;
        
        std::string input_file;
        std::cout << file_prompt;
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
        
        std::cout << "\n";
        print_section_header("FILE INFORMATION");
        print_labeled_value("Input file", emphasis(input_file));
        print_labeled_value("Output file", emphasis(output_file));
        
        // Prompt for password securely
        std::string password_prompt = is_encrypt ? 
            "\nEnter encryption password" COLOR_MAGENTA " (input hidden)" COLOR_RESET ": " :
            "\nEnter decryption password" COLOR_MAGENTA " (input hidden)" COLOR_RESET ": ";
        
        std::string password = get_password_from_user(password_prompt);
        
        std::cout << "\n";
        
        if (is_encrypt) {
            print_progress("Encrypting file with AES-256-GCM...");
            encrypt_file(input_file, output_file, password);
            std::cout << "\n";
            print_success("Encryption completed successfully!");
            print_labeled_value("Output", emphasis(output_file));
            print_feature("File is now encrypted and authenticated");
        } else {
            print_progress("Decrypting file...");
            decrypt_file(input_file, output_file, password);
            std::cout << "\n";
            print_success("Decryption completed successfully!");
            print_labeled_value("Output", emphasis(output_file));
            print_feature("File integrity verified with GCM tag");
        }
        
        print_labeled_value("Status", success_emphasis("COMPLETE"));
        std::cout << "\n";
        
        // Clear password from memory
        for (size_t i = 0; i < password.size(); ++i) {
            password[i] = '\0';
        }
        password.clear();
        
        return 0;
        
    } catch (const std::runtime_error& e) {
        std::cout << "\n";
        print_error(std::string(e.what()));
        print_warning("Operation failed - please check your input and try again");
        return 1;
    } catch (const std::exception& e) {
        std::cout << "\n";
        print_error(std::string("UNEXPECTED ERROR: ") + e.what());
        print_warning("An unexpected error occurred");
        return 1;
    }
}
