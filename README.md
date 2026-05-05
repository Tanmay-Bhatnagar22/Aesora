# Aesora - Secure File Encryption Tool

## Overview

**Aesora** is a command-line file encryption tool built in C++ that provides military-grade file protection using authenticated encryption.

### Key Features

- **AES-256-GCM Encryption**: Authenticated encryption providing both confidentiality and integrity
- **PBKDF2-HMAC-SHA256 Key Derivation**: 310,000 iterations resist brute-force attacks
- **Binary-Safe**: Encrypts any file type (images, archives, PDFs, etc.)
- **Secure Password Handling**: Passwords never exposed in process listings or shell history
- **Integrity Verification**: GCM authentication tag detects tampering
- **Cross-Platform**: Works on Windows, macOS, and Linux

---

## Quick Start

### Installation

**Prerequisites:**
- C++17 compiler (GCC, Clang, MSVC)
- OpenSSL development libraries (libssl-dev)
- CMake 3.12+

**Build:**
```bash
cd Aesora
mkdir build && cd build
cmake ..
make
```

**Executable:** `./aesora` (or `aesora.exe` on Windows)

### Usage

**Encrypt a file:**
```bash
./aesora encrypt document.pdf document.pdf.aesora
```
Password will be prompted interactively.

**Decrypt a file:**
```bash
./aesora decrypt document.pdf.aesora document.pdf
```
Enter the same password used during encryption.

---

## Technical Architecture

### Modular Design

```
include/
├── crypto_utils.h      # Cryptographic operations (PBKDF2, AES-256-GCM)
├── file_utils.h        # File I/O (binary-safe reading/writing)
├── encrypt.h           # File encryption orchestration
└── decrypt.h           # File decryption orchestration

src/
├── main.cpp            # CLI interface and password handling
├── encrypt.cpp         # Encryption implementation
├── decrypt.cpp         # Decryption implementation
├── crypto_utils.cpp    # OpenSSL wrapper functions
└── file_utils.cpp      # File operations

tests/
└── test_aesora.cpp     # Comprehensive test suite
```

### File Format

Aesora encrypted files have a specific binary format:

```
[8 bytes: Magic Header "AESORA\0\0"]
[16 bytes: Random Salt]
[12 bytes: Random IV]
[Variable: Encrypted Plaintext]
[16 bytes: GCM Authentication Tag]
```

**Minimum encrypted file size:** 52 bytes (empty plaintext)

---

## Security Design

### Cryptographic Primitives

| Component | Algorithm | Justification |
|-----------|-----------|---------------|
| Encryption | AES-256-GCM | NIST-approved, single-pass authenticated encryption |
| Key Derivation | PBKDF2-HMAC-SHA256 | NIST-approved, 310,000 iterations (~300ms/guess) |
| Random Generation | OpenSSL RAND_bytes | Cryptographically secure OS entropy pool |
| Authentication | GCM Tag (16 bytes) | Detects any tampering, prevents padding oracle attacks |

### Threat Mitigation

| Threat | Mitigation |
|--------|-----------|
| **Brute-force password attacks** | PBKDF2 with 310,000 iterations (~300ms per attempt) |
| **Rainbow table attacks** | Random 16-byte salt unique per password |
| **Replayed/predictable IVs** | Random 12-byte IV for each encryption |
| **Data tampering** | GCM authentication tag verified during decryption |
| **Password exposure** | Interactive terminal prompt (not CLI argument) |
| **Key in memory** | Sensitive data cleared with volatile writes |
| **Weak passwords** | PBKDF2 partially mitigates weak passwords (attacker still needs to crack) |

### Security Considerations

1. **Password Strength:** Aesora cannot protect weak passwords. Users should use:
   - Minimum 12 characters
   - Mix of uppercase, lowercase, digits, special characters
   - Unique per file/use case

2. **Physical Security:** No protection against:
   - Cold-boot attacks (RAM recovery)
   - Side-channel attacks (timing, power analysis)
   - Malware with OS-level access
   - Quantum computers (future threat to RSA/ECC, not relevant to AES/PBKDF2)

3. **File Metadata:** Aesora does NOT encrypt:
   - Filename
   - File timestamps
   - File permissions
   - Directory structure

---

## Usage Examples

### Example 1: Encrypt a PDF document

```bash
$ ./aesora encrypt financial_report.pdf financial_report.pdf.aesora
Enter password for encryption: [password hidden]
Encryption successful!
Input file: financial_report.pdf
Output file: financial_report.pdf.aesora
Original size: 2457600 bytes
Encrypted size: 2457652 bytes
Overhead: 52 bytes
```

### Example 2: Decrypt and verify

```bash
$ ./aesora decrypt financial_report.pdf.aesora financial_report.pdf
Enter password for decryption: [password hidden]
Decryption successful!
Encrypted file: financial_report.pdf.aesora
Decrypted file: financial_report.pdf
Plaintext size: 2457600 bytes
Encryption overhead: 52 bytes
```

### Example 3: Encrypt with wrong password attempt

```bash
$ ./aesora decrypt financial_report.pdf.aesora output.pdf
Enter password for decryption: [wrong password]
ERROR: Decryption failed: Authentication tag verification failed. 
Possible causes: wrong password or file was corrupted/modified.
```

---

## Building & Testing

### Build from Source

```bash
# Clone and enter directory
cd Aesora
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make

# Run tests (if compiled)
ctest --verbose
```

### Running Tests

```bash
./test_aesora
```

Tests cover:
- Basic encryption/decryption roundtrips
- Wrong password detection
- Binary file support
- Empty files
- Large files (10MB+)
- File corruption detection
- Special characters in passwords
- Key derivation consistency

---

## Performance

Typical performance on modern CPU (with AES-NI acceleration):

| Operation | Time | Notes |
|-----------|------|-------|
| Key Derivation (PBKDF2) | 300-500ms | Depends on password length |
| Encryption (AES-256-GCM) | 100-200 MB/sec | Limited by disk I/O on modern systems |
| Decryption (AES-256-GCM) | 100-200 MB/sec | Includes authentication verification |

**Example:** 100MB file encryption = ~300ms (PBKDF2) + ~500ms (encryption) = ~800ms total

---

## OpenSSL Configuration

### Windows

If OpenSSL is not found during CMake configuration:

1. Download: https://slproweb.com/products/Win32OpenSSL.html
2. Install to `C:\Program Files\OpenSSL-Win64`
3. Set environment variable: `OPENSSL_ROOT_DIR=C:\Program Files\OpenSSL-Win64`

Alternatively, use vcpkg:
```bash
vcpkg install openssl:x64-windows
```

### Linux

```bash
# Ubuntu/Debian
sudo apt-get install libssl-dev

# Fedora/RHEL
sudo dnf install openssl-devel

# Alpine
apk add openssl-dev
```

### macOS

```bash
brew install openssl
```

---

## Security Audit Checklist

- [x] Uses NIST-approved cryptography (AES-256, PBKDF2-HMAC-SHA256)
- [x] Authenticated encryption (AES-256-GCM) prevents tampering
- [x] High iteration PBKDF2 resists brute-force
- [x] Random salts prevent rainbow tables
- [x] Random IVs prevent patterns
- [x] Passwords not stored or logged
- [x] Sensitive data cleared from memory
- [x] No hardcoded keys or constants
- [x] Proper error handling without information leakage
- [x] Supports binary files correctly
- [x] File format validation prevents processing non-Aesora files

---

## Limitations & Future Work

### Current Limitations

1. **Entire file in memory:** Large files (>available RAM) not supported
2. **No key file support:** Only password-based encryption
3. **No metadata encryption:** Filenames, timestamps visible
4. **No multi-file archive:** Single file at a time

### Potential Enhancements

1. **Streaming encryption:** Handle files larger than RAM
2. **Key files:** Support .pem private keys for additional security
3. **Batch encryption:** Encrypt multiple files with one command
4. **Password-protected archives:** Create encrypted .tar.gz files
5. **CLI progress indicator:** Show encryption progress for large files
6. **Metadata encryption:** Optional filename/timestamp obfuscation

---

## License

See LICENSE file for details.

---

## Security Reporting

If you discover a security vulnerability, please do NOT open a public issue. Instead:
1. Document the vulnerability
2. Report privately to the maintainer
3. Allow time for a fix before public disclosure

---

## References

- NIST SP 800-132: PBKDF2 Specification
- NIST SP 800-38D: GCM Mode Specification
- RFC 5116: CRYPTOGRAPHIC ALGORITHM INTERFACE AND USAGE
- OpenSSL Documentation: https://www.openssl.org/docs/

---

## FAQ

**Q: Can I use Aesora for compliance (HIPAA, PCI-DSS, etc.)?**  
A: Aesora uses NIST-approved algorithms suitable for regulated environments. However, audit encryption usage and consult compliance requirements.

**Q: Is the tool open-source?**  
A: Yes, see LICENSE file.

**Q: Can I integrate Aesora into my application?**  
A: Yes, the modular design allows linking crypto libraries. See `include/encrypt.h` and `include/decrypt.h`.

**Q: What if I forget my password?**  
A: There is NO password recovery. Lost passwords = lost data (intentional security design). Use password managers.

**Q: How do I verify file integrity?**  
A: Aesora uses GCM authentication tag to verify ciphertext during decryption. If decryption succeeds, file is authentic.

---

**Last Updated:** May 5, 2026  
**Version:** 1.0 Production
