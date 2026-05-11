/**
 * @file terminal_colors.h
 * @brief Terminal color and styling utilities for professional CLI output
 * 
 * This module provides ANSI color codes with Windows compatibility.
 * Supports Windows Terminal, PowerShell, VS Code terminal, and Unix terminals.
 * 
 * SECURITY NOTE:
 * Color codes are purely visual and do not affect cryptographic operations.
 * All color output uses standard ANSI escape sequences.
 */

#ifndef TERMINAL_COLORS_H
#define TERMINAL_COLORS_H

#include <iostream>
#include <string>

#ifdef _WIN32
    #include <windows.h>
#endif

// ============================================================================
// ANSI COLOR CODES
// ============================================================================

// Text Color Codes
#define COLOR_RESET          "\033[0m"
#define COLOR_BOLD           "\033[1m"
#define COLOR_DIM            "\033[2m"
#define COLOR_ITALIC         "\033[3m"
#define COLOR_UNDERLINE      "\033[4m"

// Standard Colors
#define COLOR_RED            "\033[31m"
#define COLOR_GREEN          "\033[32m"
#define COLOR_YELLOW         "\033[33m"
#define COLOR_BLUE           "\033[34m"
#define COLOR_MAGENTA        "\033[35m"
#define COLOR_CYAN           "\033[36m"
#define COLOR_WHITE          "\033[37m"

// Bright Colors
#define COLOR_BRIGHT_RED     "\033[91m"
#define COLOR_BRIGHT_GREEN   "\033[92m"
#define COLOR_BRIGHT_YELLOW  "\033[93m"
#define COLOR_BRIGHT_BLUE    "\033[94m"
#define COLOR_BRIGHT_MAGENTA "\033[95m"
#define COLOR_BRIGHT_CYAN    "\033[96m"
#define COLOR_BRIGHT_WHITE   "\033[97m"

// Background Colors
#define BG_RED               "\033[41m"
#define BG_GREEN             "\033[42m"
#define BG_YELLOW            "\033[43m"
#define BG_BLUE              "\033[44m"
#define BG_MAGENTA           "\033[45m"
#define BG_CYAN              "\033[46m"
#define BG_WHITE             "\033[47m"

// Professional Color Combinations
#define COLOR_BOLD_CYAN      COLOR_BOLD COLOR_CYAN
#define COLOR_BOLD_GREEN     COLOR_BOLD COLOR_GREEN
#define COLOR_BOLD_RED       COLOR_BOLD COLOR_RED
#define COLOR_BOLD_YELLOW    COLOR_BOLD COLOR_YELLOW

// ============================================================================
// INDICATOR SYMBOLS WITH COLORS
// ============================================================================

#define SUCCESS_INDICATOR    COLOR_BOLD_GREEN "[+]" COLOR_RESET
#define ERROR_INDICATOR      COLOR_BOLD_RED "[-]" COLOR_RESET
#define INFO_INDICATOR       COLOR_BOLD_CYAN "[*]" COLOR_RESET
#define WARNING_INDICATOR    COLOR_BOLD_YELLOW "[!]" COLOR_RESET
#define LOCK_INDICATOR       COLOR_BOLD_MAGENTA "[🔐]" COLOR_RESET

// ============================================================================
// STYLED STRINGS FOR SECURITY FEATURES
// ============================================================================

#define FEATURE_AES          COLOR_YELLOW "AES-256-GCM" COLOR_RESET
#define FEATURE_PBKDF2       COLOR_YELLOW "PBKDF2-HMAC-SHA256" COLOR_RESET
#define FEATURE_SALT         COLOR_YELLOW "Random Salt" COLOR_RESET
#define FEATURE_IV           COLOR_YELLOW "Random IV" COLOR_RESET
#define FEATURE_GCM_TAG      COLOR_YELLOW "GCM Tag" COLOR_RESET

// ============================================================================
// WINDOWS ANSI SUPPORT DETECTION AND ENABLING
// ============================================================================

/**
 * Enables ANSI escape sequences on Windows terminals.
 * 
 * On Windows, the ENABLE_VIRTUAL_TERMINAL_PROCESSING flag must be set
 * to enable ANSI escape codes. This function handles:
 * - Windows 10 and later with Windows Terminal or ConEmu
 * - PowerShell 7+
 * - VS Code integrated terminal
 * - Falls back gracefully on older Windows versions
 * 
 * On Unix-like systems (Linux, macOS), ANSI codes work natively.
 * 
 * @return true if ANSI support is available, false otherwise
 */
inline bool enable_ansi_support() {
    #ifdef _WIN32
    {
        // Get standard output handle
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        
        if (hOut == INVALID_HANDLE_VALUE) {
            return false;
        }
        
        // Get current console mode
        DWORD dwMode = 0;
        if (!GetConsoleMode(hOut, &dwMode)) {
            return false;
        }
        
        // Enable VIRTUAL_TERMINAL_PROCESSING flag
        // Note: This constant may already be defined in windows.h, so we check
        #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
            #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
        #endif
        
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        
        // Apply the new mode
        if (!SetConsoleMode(hOut, dwMode)) {
            // Fallback: Still try to use colors even if SetConsoleMode fails
            // Some environments (like CI/CD) may not support this but colors still work
            return false;
        }
        
        return true;
    }
    #else
    {
        // Unix-like systems (Linux, macOS) support ANSI natively
        return true;
    }
    #endif
}

// ============================================================================
// HELPER FUNCTIONS FOR COLORED OUTPUT
// ============================================================================

/**
 * Prints a styled success message.
 * Format: [+] message
 * Color: Green
 */
inline void print_success(const std::string& message) {
    std::cout << SUCCESS_INDICATOR << " " << message << "\n";
}

/**
 * Prints a styled error message.
 * Format: [-] message
 * Color: Red
 */
inline void print_error(const std::string& message) {
    std::cerr << ERROR_INDICATOR << " " << message << "\n";
}

/**
 * Prints a styled info message.
 * Format: [*] message
 * Color: Cyan
 */
inline void print_info(const std::string& message) {
    std::cout << INFO_INDICATOR << " " << message << "\n";
}

/**
 * Prints a styled warning message.
 * Format: [!] message
 * Color: Yellow
 */
inline void print_warning(const std::string& message) {
    std::cout << WARNING_INDICATOR << " " << message << "\n";
}

/**
 * Prints a styled security feature.
 * Used in banners and feature lists.
 */
inline void print_feature(const std::string& feature) {
    std::cout << "  " << SUCCESS_INDICATOR << " " << feature << "\n";
}

/**
 * Prints a styled banner/header.
 * Format: Bold Cyan text centered
 */
inline void print_banner_line(const std::string& text) {
    std::cout << COLOR_BOLD_CYAN << text << COLOR_RESET << "\n";
}

/**
 * Prints a section header (yellow with formatting).
 */
inline void print_section_header(const std::string& title) {
    std::cout << "\n" << COLOR_BOLD_YELLOW << title << ":" << COLOR_RESET << "\n";
}

/**
 * Prints a prompt with magenta styling.
 */
inline void print_prompt(const std::string& prompt) {
    std::cout << COLOR_MAGENTA << prompt << COLOR_RESET;
    std::cout.flush();
}

/**
 * Prints a labeled value (for status displays).
 * Format: label: value
 */
inline void print_labeled_value(const std::string& label, const std::string& value) {
    std::cout << "  " << COLOR_CYAN << label << COLOR_RESET << ": " << value << "\n";
}

/**
 * Prints emphasis text in bold cyan.
 */
inline std::string emphasis(const std::string& text) {
    return COLOR_BOLD_CYAN + text + COLOR_RESET;
}

/**
 * Prints emphasis text in bold yellow.
 */
inline std::string warning_emphasis(const std::string& text) {
    return COLOR_BOLD_YELLOW + text + COLOR_RESET;
}

/**
 * Prints emphasis text in bold green.
 */
inline std::string success_emphasis(const std::string& text) {
    return COLOR_BOLD_GREEN + text + COLOR_RESET;
}

/**
 * Prints emphasis text in bold red.
 */
inline std::string error_emphasis(const std::string& text) {
    return COLOR_BOLD_RED + text + COLOR_RESET;
}

// ============================================================================
// PROGRESS INDICATORS
// ============================================================================

/**
 * Prints a progress message for operations.
 * Example: "Encrypting file..."
 */
inline void print_progress(const std::string& message) {
    std::cout << COLOR_CYAN << "⟳ " << message << COLOR_RESET << "\n";
}

/**
 * Prints a separator line for visual organization.
 */
inline void print_separator() {
    std::cout << COLOR_DIM << "==================================================" << COLOR_RESET << "\n";
}

#endif // TERMINAL_COLORS_H
