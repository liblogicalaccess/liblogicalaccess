cmake_minimum_required(VERSION 3.10)
project(basic LANGUAGES CXX)

# ========================================
# Common libraries
# ========================================

set(LIBLOGICALACCESS_COMMON_LIBRARIES
    logicalaccess
    iso7816cards
    desfirecards
    iso7816readers
    pcscreaders
)

# ========================================
# Executable creation helper
# ========================================

function(add_basic_test name)
    add_executable(${name} ${name}.cpp)

    target_link_libraries(${name}
        PRIVATE
            ${LIBLOGICALACCESS_COMMON_LIBRARIES}
            ${ARGN}
    )
endfunction()

# ========================================
# Test suites
# ========================================

add_basic_test(basic)
add_basic_test(basic_pki)
add_basic_test(basic_desfire_sam samavcards)
add_basic_test(basic_samkeyentry samavcards)

add_basic_test(basic_duox)
add_basic_test(basic_duox_aes_authentication)
add_basic_test(basic_duox_transaction_management)
add_basic_test(basic_duox_crypto_api)
add_basic_test(basic_duox_ev_charging)

add_basic_test(basic_certificate)
add_basic_test(basic_duoxecc)
add_basic_test(basic_authenticate)

# Factory application is expected on MIFARE DUOX for EV Charging but not on Standard MIFARE DUOX
# add_basic_test(basic_duox_ev_charging_readonly)

# ========================================
# Installation
# ========================================

install(
    TARGETS
        basic
        basic_pki
        basic_desfire_sam
        basic_samkeyentry
        basic_duox
        basic_duox_aes_authentication
        basic_duox_transaction_management
        basic_duox_crypto_api
        basic_duox_ev_charging
        # basic_duox_ev_charging_readonly
        basic_certificate
        basic_duoxecc
        basic_authenticate

    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib${LIB_SUFFIX}
    ARCHIVE DESTINATION lib${LIB_SUFFIX}
)