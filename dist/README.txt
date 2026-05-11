================================================================================
                    AESORA - SECURE FILE ENCRYPTION TOOL
                              Version 1.0.0
================================================================================

OVERVIEW
--------
Aesora is a command-line file encryption tool that provides military-grade 
file protection using authenticated encryption (AES-256-GCM).

KEY FEATURES
------------
- AES-256-GCM Encryption: Authenticated encryption for confidentiality & integrity
- PBKDF2-HMAC-SHA256 Key Derivation: 310,000 iterations resist brute-force attacks
- Binary-Safe: Encrypts any file type (images, archives, PDFs, etc.)
- Secure Password Handling: Passwords never exposed in process listings
- Integrity Verification: GCM authentication detects tampering
- Cross-Platform: Works on Windows, macOS, and Linux

INSTALLATION
------------

Windows (Using Installer):
  1. Run Aesora-1.0.0-Setup.exe
  2. Follow the installation wizard
  3. Optionally add to PATH for command-line access
  4. Installation complete!

Windows (Manual):
  1. Copy aesora.exe to your preferred directory
  2. Optionally add the directory to your Windows PATH environment variable
  3. Open Command Prompt and type: aesora --help

macOS / Linux (Manual):
  1. Copy the aesora executable to /usr/local/bin or ~/bin
  2. Make it executable: chmod +x aesora
  3. Run: aesora --help

QUICK START
-----------

Encrypt a file:
  aesora encrypt document.pdf document.pdf.aesora

Decrypt a file:
  aesora decrypt document.pdf.aesora document.pdf

You will be prompted to enter a password for encryption/decryption.

USAGE EXAMPLES
--------------

Encrypt a Word document:
  aesora encrypt Resume.docx Resume.docx.aesora

Encrypt an archive:
  aesora encrypt backup.zip backup.zip.aesora

Decrypt back to original:
  aesora decrypt backup.zip.aesora backup.zip

SECURITY NOTES
--------------
- Passwords are NEVER stored or logged
- Use strong, memorable passwords (12+ characters recommended)
- The .aesora extension indicates an encrypted file
- Keep your passwords safe - lost passwords cannot be recovered
- Files are encrypted with AES-256-GCM (authenticated encryption)
- Integrity check detects if a file has been tampered with

SYSTEM REQUIREMENTS
-------------------
- Windows 7 or later (64-bit)
- macOS 10.12 or later
- Linux kernel 3.10 or later
- OpenSSL libraries (usually pre-installed on macOS/Linux)
- ~10 MB disk space for installation

TROUBLESHOOTING
---------------

"Command not found" error:
  - Ensure Aesora is in your PATH
  - On Windows, add the installation directory to Environment Variables
  - Or use the full path: C:\Program Files\Aesora\bin\aesora.exe

"OpenSSL not found" error (Linux/macOS):
  - Install OpenSSL: sudo apt-get install libssl-dev (Ubuntu/Debian)
  - Or: brew install openssl (macOS)

"Permission denied" error (macOS/Linux):
  - Make the file executable: chmod +x aesora
  - Then try again

File encryption failing:
  - Ensure you have read permission on the input file
  - Ensure you have write permission in the output directory
  - Check available disk space
  - Try encrypting a smaller test file first

UNINSTALLATION
--------------

Windows:
  1. Go to Settings > Apps > Apps & Features
  2. Find "Aesora" in the list
  3. Click Uninstall

Manual / Linux / macOS:
  Simply delete the aesora executable and related files

SUPPORT & DOCUMENTATION
-----------------------
For detailed documentation, visit: https://github.com/yourusername/aesora
For bug reports and feature requests: https://github.com/yourusername/aesora/issues
For discussions and help: https://github.com/yourusername/aesora/discussions

LICENSE
-------
This software is distributed under the terms specified in the LICENSE file
included with this distribution. See LICENSE.txt for details.

DISCLAIMER
----------
This software is provided "AS IS" WITHOUT WARRANTY OF ANY KIND. The author
disclaims all liability for any damage or loss resulting from the use of
this software. Users are responsible for maintaining backups of important
files before encryption.

================================================================================
Thank you for using Aesora!
================================================================================
