cmake_minimum_required(VERSION 3.12)
project(Aesora)

# ============================================================================
# PROJECT CONFIGURATION
# ============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# ============================================================================
# FIND OPENSSL (REQUIRED FOR CRYPTOGRAPHY)
# ============================================================================

# OpenSSL is required for:
# - PBKDF2-HMAC-SHA256 key derivation
# - AES-256-GCM encryption
# - Secure random number generation

find_package(OpenSSL REQUIRED)

if(NOT OPENSSL_FOUND)
    message(FATAL_ERROR
        "OpenSSL not found!\n"
        "Please install OpenSSL development libraries:\n"
        "  Ubuntu/Debian: sudo apt-get install libssl-dev\n"
        "  macOS: brew install openssl\n"
        "  Windows: Download from https://slproweb.com/products/Win32OpenSSL.html\n"
        "           or use vcpkg: vcpkg install openssl:x64-windows"
    )
endif()

message(STATUS "OpenSSL found: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")

# ============================================================================
# INCLUDE DIRECTORIES
# ============================================================================

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${OPENSSL_INCLUDE_DIR})

# ============================================================================
# SOURCE FILES
# ============================================================================

set(AESORA_CORE_SOURCES
    src/encrypt.cpp
    src/decrypt.cpp
    src/crypto_utils.cpp
    src/file_utils.cpp
)

set(AESORA_MAIN
    src/main.cpp
)

set(AESORA_HEADERS
    include/encrypt.h
    include/decrypt.h
    include/crypto_utils.h
    include/file_utils.h
)

# ============================================================================
# BUILD EXECUTABLE
# ============================================================================

add_executable(aesora ${AESORA_MAIN} ${AESORA_CORE_SOURCES} ${AESORA_HEADERS})

# Link OpenSSL libraries
target_link_libraries(aesora PRIVATE OpenSSL::Crypto)

# ============================================================================
# INSTALLATION
# ============================================================================

install(TARGETS aesora DESTINATION bin)

# ============================================================================
# OPTIONAL: ENABLE TESTING
# ============================================================================

enable_testing()

# Add test executable (if tests/test_aesora.cpp exists)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_aesora.cpp")
    add_executable(test_aesora tests/test_aesora.cpp ${AESORA_CORE_SOURCES} ${AESORA_HEADERS})
    target_link_libraries(test_aesora PRIVATE OpenSSL::Crypto)
    add_test(NAME AesoraTests COMMAND test_aesora)
endif()

# ============================================================================
# PRINT CONFIGURATION SUMMARY
# ============================================================================

message(STATUS "")
message(STATUS "=== Aesora Build Configuration ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "OpenSSL: ${OPENSSL_VERSION}")
message(STATUS "Source files: ${AESORA_SOURCES}")
message(STATUS "")
