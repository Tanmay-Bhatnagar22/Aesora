cmake_minimum_required(VERSION 3.12)
project(Aesora)

# ============================================================================
# PROJECT CONFIGURATION
# ============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Option to enable static linking for portability
option(BUILD_STATIC_EXECUTABLE "Build a fully static executable for portability" OFF)

if(BUILD_STATIC_EXECUTABLE)
    # Static linking flags
    if(MSVC)
        # For MSVC: link against static runtime
        string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    else()
        # For GCC/Clang: enable static linking
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    endif()
    message(STATUS "Building static executable for portability")
endif()

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
# RESOURCE FILES (CROSS-PLATFORM ICONS)
# ============================================================================

set(AESORA_RESOURCES)

if(WIN32)
    # Windows: Use .rc resource file
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.rc")
        set(AESORA_RESOURCES ${AESORA_RESOURCES} Resources/icon.rc)
    endif()
elseif(APPLE)
    # macOS: Use .icns bundle icon
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.icns")
        set(AESORA_RESOURCES ${AESORA_RESOURCES} Resources/icon.icns)
    endif()
endif()

# ============================================================================
# BUILD EXECUTABLE
# ============================================================================

add_executable(aesora ${AESORA_MAIN} ${AESORA_CORE_SOURCES} ${AESORA_HEADERS} ${AESORA_RESOURCES})

# macOS: Configure bundle icon
if(APPLE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.icns")
    set_target_properties(aesora PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE icon.icns
    )
    set_source_files_properties(Resources/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()

# Link OpenSSL libraries
if(BUILD_STATIC_EXECUTABLE AND NOT MSVC)
    # For static linking on Linux/macOS: use static libraries
    target_link_libraries(aesora PRIVATE ${OPENSSL_CRYPTO_LIBRARY})
else()
    # Default: dynamic linking
    target_link_libraries(aesora PRIVATE OpenSSL::Crypto)
endif()

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
    add_executable(test_aesora tests/test_aesora.cpp ${AESORA_CORE_SOURCES} ${AESORA_HEADERS} ${AESORA_RESOURCES})
    
    # macOS: Configure bundle icon for test executable
    if(APPLE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon.icns")
        set_target_properties(test_aesora PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_ICON_FILE icon.icns
        )
        set_source_files_properties(Resources/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    endif()
    
    if(BUILD_STATIC_EXECUTABLE AND NOT MSVC)
        target_link_libraries(test_aesora PRIVATE ${OPENSSL_CRYPTO_LIBRARY})
    else()
        target_link_libraries(test_aesora PRIVATE OpenSSL::Crypto)
    endif()
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
