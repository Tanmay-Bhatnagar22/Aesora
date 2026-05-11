================================================================================
                    AESORA - SECURE FILE ENCRYPTION TOOL
================================================================================

PROJECT OVERVIEW
----------------
Aesora is a command-line file encryption tool built in C++ that provides 
military-grade file protection using authenticated encryption (AES-256-GCM).

It encrypts any file type securely while ensuring both confidentiality and 
integrity protection.

================================================================================
KEY FEATURES
================================================================================

- AES-256-GCM Encryption: Authenticated encryption providing confidentiality 
  and integrity verification
  
- PBKDF2-HMAC-SHA256 Key Derivation: 310,000 iterations for strong 
  resistance against brute-force attacks
  
- Binary-Safe: Encrypts any file type (images, archives, PDFs, documents, etc.)

- Secure Password Handling: Passwords never exposed in process listings or 
  shell history

- Integrity Verification: GCM authentication tag automatically detects any 
  tampering or file corruption

- Cross-Platform: Works on Windows, macOS, and Linux

================================================================================
QUICK START
================================================================================

PREREQUISITES:
  - C++17 compatible compiler (GCC, Clang, or MSVC)
  - OpenSSL development libraries (libssl-dev)
  - CMake 3.12 or higher

BUILDING THE PROJECT:
  
  1. Navigate to the project directory:
     cd Aesora
     
  2. Create and enter build directory:
     mkdir build && cd build
     
  3. Configure with CMake:
     cmake ..
     
  4. Build the project:
     cmake --build . (Windows)
     make (Linux/macOS)
     
  5. The executable will be available as:
     - aesora.exe (Windows)
     - ./aesora (Linux/macOS)

USAGE:
  
  Encrypt a file:
    aesora encrypt input_file encrypted_file
    (Password will be prompted interactively)
    
  Decrypt a file:
    aesora decrypt encrypted_file output_file
    (Enter the same password used during encryption)

Example:
  aesora encrypt document.pdf document.pdf.aesora
  aesora decrypt document.pdf.aesora document.pdf

================================================================================
PROJECT STRUCTURE
================================================================================

include/
  crypto_utils.h       - Cryptographic operations (PBKDF2, AES-256-GCM)
  file_utils.h         - File I/O operations (binary-safe)
  encrypt.h            - File encryption orchestration
  decrypt.h            - File decryption orchestration
  terminal_colors.h    - Terminal color formatting utilities

src/
  main.cpp             - Command-line interface and password handling
  encrypt.cpp          - Encryption implementation
  decrypt.cpp          - Decryption implementation
  crypto_utils.cpp     - OpenSSL wrapper functions
  file_utils.cpp       - File I/O operations

tests/
  test_aesora.cpp      - Comprehensive test suite

CMakeLists.txt         - Build configuration
installer/
  installer.iss        - Windows installer script

================================================================================
ENCRYPTED FILE FORMAT
================================================================================

Aesora encrypted files follow this binary structure:

  [8 bytes]   : Magic Header "AESORA" + 2 null bytes
  [16 bytes]  : Random Salt (for key derivation)
  [12 bytes]  : Random Initialization Vector (IV)
  [Variable]  : Encrypted Plaintext
  [16 bytes]  : GCM Authentication Tag

Minimum encrypted file size: 52 bytes (empty plaintext)

================================================================================
SECURITY DESIGN
================================================================================

CRYPTOGRAPHIC STANDARDS USED:

  Encryption Algorithm    : AES-256-GCM
  Key Derivation Function : PBKDF2-HMAC-SHA256 (310,000 iterations)
  Random Generation       : OpenSSL RAND_bytes
  Authentication          : GCM 16-byte Tag

SECURITY PROPERTIES:

  - NIST-approved algorithms for both encryption and key derivation
  - Single-pass authenticated encryption (prevents tampering)
  - ~300ms per password guess (strong resistance to brute force)
  - Cryptographically secure random values for salt and IV
  - GCM mode prevents padding oracle and other known attacks
  - Each file uses unique salt and IV for additional security

================================================================================
SYSTEM REQUIREMENTS
================================================================================

MINIMUM:
  - 64-bit operating system (Windows, Linux, or macOS)
  - 50 MB free disk space
  - 256 MB RAM

RECOMMENDED:
  - Multi-core processor for faster encryption of large files
  - SSD for improved performance

SUPPORTED PLATFORMS:
  - Windows 7 and later (32/64-bit)
  - Ubuntu 18.04 and later
  - Debian 9 and later
  - macOS 10.12 and later
  - CentOS 7 and later

================================================================================
TESTING
================================================================================

To run the test suite:

  1. Build the project (see BUILDING THE PROJECT section)
  2. From the build directory:
     ctest (Linux/macOS)
     ctest -C Debug (Windows)

The test suite validates:
  - Encryption and decryption correctness
  - Key derivation functionality
  - File I/O operations
  - Integrity verification
  - Edge cases and error handling

================================================================================
INSTALLATION (WINDOWS)
================================================================================

A Windows installer is provided in the installer/ directory:

  1. Download or build aesora.exe
  2. Run installer.iss with Inno Setup to create an installer
  3. The installer will add Aesora to your program files and path

Alternatively, use the executable directly without installation.

================================================================================
LICENSE
================================================================================

See LICENSE file in the project root for license information.

================================================================================
SUPPORT & DOCUMENTATION
================================================================================

For detailed documentation, refer to:
  - README.md (extended documentation)
  - Source code comments and inline documentation
  - The test suite (tests/test_aesora.cpp) for usage examples

================================================================================
IMPORTANT SECURITY NOTES
================================================================================

1. USE STRONG PASSWORDS
   - Use passwords with at least 12-16 characters
   - Mix uppercase, lowercase, numbers, and special characters
   - Avoid dictionary words and personal information

2. KEEP PASSWORDS SECURE
   - Never share your encryption passwords
   - Do not store passwords in plain text
   - Use a password manager if handling multiple files

3. BACKUP ENCRYPTED FILES
   - Keep encrypted backups in secure locations
   - Verify encrypted file integrity regularly
   - If a file is corrupted, decryption will fail

4. VERIFY FILE INTEGRITY
   - Aesora automatically verifies integrity via GCM tags
   - Any tampering or corruption is detected and reported
   - Do not trust corrupted encrypted files

================================================================================
                         End of README.txt
================================================================================
