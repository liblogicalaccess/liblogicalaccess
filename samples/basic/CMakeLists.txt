cmake_minimum_required(VERSION 3.10)
project(basic LANGUAGES CXX)

# Common libraries
set(LIBLOGICALACCESS_COMMON_LIBRARIES
    logicalaccess
    iso7816cards
    desfirecards
    iso7816readers
    pcscreaders
)

# basic
add_executable(basic
    basic.cpp
)

target_link_libraries(basic
    PRIVATE
        ${LIBLOGICALACCESS_COMMON_LIBRARIES}
)

# basic_pki
add_executable(basic_pki
    basic_pki.cpp
)

target_link_libraries(basic_pki
    PRIVATE
        ${LIBLOGICALACCESS_COMMON_LIBRARIES}
)

# basic_desfire_sam
add_executable(basic_desfire_sam
    basic_desfire_sam.cpp
)

target_link_libraries(basic_desfire_sam
    PRIVATE
        ${LIBLOGICALACCESS_COMMON_LIBRARIES}
        samavcards
)

# basic_samkeyentry
add_executable(basic_samkeyentry
    basic_samkeyentry.cpp
)

target_link_libraries(basic_samkeyentry
    PRIVATE
        ${LIBLOGICALACCESS_COMMON_LIBRARIES}
        samavcards
)

# basic_duox
add_executable(basic_duox
    basic_duox.cpp
)

target_link_libraries(basic_duox
    PRIVATE
        ${LIBLOGICALACCESS_COMMON_LIBRARIES}
)

# basic_duox_aes_authentication
add_executable(basic_duox_aes_authentication
    basic_duox_aes_authentication.cpp
)

target_link_libraries(basic_duox_aes_authentication
    PRIVATE
        ${LIBLOGICALACCESS_COMMON_LIBRARIES}
)

# basic_duox_transaction_management
add_executable(basic_duox_transaction_management
    basic_duox_transaction_management.cpp
)

target_link_libraries(basic_duox_transaction_management
    PRIVATE
        ${LIBLOGICALACCESS_COMMON_LIBRARIES}
)

# basic_certificate
add_executable(basic_certificate
    basic_certificate.cpp
)

target_link_libraries(basic_certificate
    PRIVATE
        ${LIBLOGICALACCESS_COMMON_LIBRARIES}
)

# basic_duoxecc
add_executable(basic_duoxecc
    basic_duoxecc.cpp
)

target_link_libraries(basic_duoxecc
    PRIVATE
        ${LIBLOGICALACCESS_COMMON_LIBRARIES}
)

# basic_authenticate
add_executable(basic_authenticate
    basic_authenticate.cpp
)

target_link_libraries(basic_authenticate
    PRIVATE
        ${LIBLOGICALACCESS_COMMON_LIBRARIES}
)

# Installation
install(
    TARGETS
        basic
        basic_pki
        basic_desfire_sam
        basic_samkeyentry
        basic_duox
        basic_duox_aes_authentication
        basic_duox_transaction_management
        basic_certificate
        basic_duoxecc
        basic_authenticate
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib${LIB_SUFFIX}
    ARCHIVE DESTINATION lib${LIB_SUFFIX}
)