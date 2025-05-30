cmake_minimum_required(VERSION 3.10)
project(logicalaccess)

# Global C++ configuration
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Some build options. Either configured through CMake directly, or fed
# by Conan options. It is important, for consistency, to keep those defaults
# in sync with conafile.py
set(LLA_BUILD_PKCS TRUE CACHE BOOL "Build PKCS11 support")

include(GenerateExportHeader)

if (NOT BUILD_TESTING STREQUAL OFF)
    find_package(GTest)
endif ()
find_package(Boost)
find_package(OpenSSL)
find_package(nlohmann_json)
if (LLA_BUILD_PKCS)
    find_package(cppkcs11)
endif ()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# We define a macro that will help us properly generate and install
# export headers for all libraries that are part of LLA.
# This macro should be called with the following preconditions:
#    + A variable HEADER_RELATIVE_INSTALL_DIR should be defined and point to a
#      valid RELATIVE path for export header installation.
#    + A single argument should be provide, which will be used as part of the export
#      macro name. If LLA_CORE is passed, the export macro name is LLA_CORE_API.
#
# The generated header is XXX_api.hpp, where XXX is lowered-case of the first parameter
# of the macro.
#
# NOTE THAT THIS MACRO NEEDS ${PROJECT_NAME} TO EVALUATE TO A TARGET NAME
macro(LLA_GENERATE_EXPORTS)
    string(TOLOWER ${ARGV0} _LOWERCASE_LIB_NAME)
    set(_EXPORT_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${_LOWERCASE_LIB_NAME}_api.hpp")

    generate_export_header(${PROJECT_NAME}
            EXPORT_MACRO_NAME "${ARGV0}_API"
            EXPORT_FILE_NAME ${_EXPORT_FILE_PATH})

    target_compile_definitions(${PROJECT_NAME} PRIVATE "${_LOWERCASE_LIB_NAME}_EXPORTS=1")
    # Disable some MSVC warning we don't care about.
    if (MSVC)
        target_compile_options(${PROJECT_NAME} PUBLIC "/wd4251")
    else()
        target_compile_options(${PROJECT_NAME} PUBLIC -W -Wall -Wno-unused-parameter -Wsign-conversion)
        if (CMAKE_BUILD_TYPE MATCHES "Debug")
            target_compile_options(logicalaccess PUBLIC -g)
        endif()
    endif ()

    # HEADER_INSTALL_DIR must be defined before calling the macro.
    install(FILES ${_EXPORT_FILE_PATH} DESTINATION ${HEADER_RELATIVE_INSTALL_DIR})
endmacro()
#### We dont use this for LLA_CORE macro.


file(GLOB LLA_RESOURCE_RC ${CMAKE_CURRENT_SOURCE_DIR}/vc++/resource.rc)
file(GLOB_RECURSE LLA_NATIVE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE LLA_NATIVE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

# Those 2 are under plugin but they arent plugins, they are
# required for LLA to link properly.
add_subdirectory(plugins/logicalaccess/plugins/llacommon)
add_subdirectory(plugins/logicalaccess/plugins/crypto)

# Create logicalaccess target
set(LIBLOGICALACCESS_LIBRARY logicalaccess)
add_library(logicalaccess SHARED ${LLA_NATIVE_SOURCES} ${LLA_NATIVE_HEADERS} ${LLA_RESOURCE_RC})

# We rely on CMake to generate an export macro for the target.
generate_export_header(logicalaccess
        EXPORT_MACRO_NAME LLA_CORE_API
        EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/include/logicalaccess/lla_core_api.hpp")
target_compile_definitions(logicalaccess PRIVATE logicalaccess_EXPORTS=1)
      
if (LLA_BOOST_ASIO_HAS_STD_STRING_VIEW)
  # Work around a boost / clang issue with include to string/view.
  # See https://github.com/boostorg/asio/issues/82
  target_compile_definitions(logicalaccess PUBLIC BOOST_ASIO_HAS_STD_STRING_VIEW=1)
endif()

# Variable containing external library that LLA needs to link against
set(LLA_ADDITIONAL_LIBRARIES_TO_LINK_AGAINST)

if (UNIX AND NOT ANDROID)
    target_compile_options(logicalaccess PUBLIC -W -Wall -Wno-unused-parameter)
    list(APPEND LLA_ADDITIONAL_LIBRARIES_TO_LINK_AGAINST dl pthread)
endif ()

target_link_libraries(logicalaccess PUBLIC
        llacommon
        logicalaccess-cryptolib
        OpenSSL::Crypto
        ${LLA_ADDITIONAL_LIBRARIES_TO_LINK_AGAINST}
        Boost::boost
        nlohmann_json::nlohmann_json
        )

# Wrong, fix /plugins/
target_include_directories(logicalaccess PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_options(logicalaccess PUBLIC -O0 -g)
endif()

install(TARGETS ${LIBLOGICALACCESS_LIBRARY}
        RUNTIME DESTINATION bin/${LIB_SUFFIX}
        LIBRARY DESTINATION lib/${LIB_SUFFIX}
        ARCHIVE DESTINATION lib/${LIB_SUFFIX})

if (MSVC)
    #Enable PDB for release
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")

    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
    SET(VERSION_PRODUCTNAME_VALUE "Liblogicalaccess")
    SET(VERSION_PROJECT_FILE_DESCRIPTION_VALUE "${VERSION_PRODUCTNAME_VALUE}\ (${TARGET_ARCH})")
    target_compile_definitions(logicalaccess PUBLIC -DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}" -DVERSION_PROJECT_FILE_DESCRIPTION_VALUE="${VERSION_PROJECT_FILE_DESCRIPTION_VALUE}" -DVERSION_FILE_AND_PRODUCT_VALUE_STRING="${LIBLOGICALACCESS_VERSION_STRING}" -DVERSION_FILE_AND_PRODUCT_VALUE=${LIBLOGICALACCESS_WINDOWS_VERSION})
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif ()

set_target_properties(${LIBLOGICALACCESS_LIBRARY} PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})
set_target_properties(${LIBLOGICALACCESS_LIBRARY} PROPERTIES LINKER_LANGUAGE CXX)

add_subdirectory(plugins/logicalaccess/plugins/cards)
add_subdirectory(plugins/logicalaccess/plugins/readers)

if (LLA_BUILD_PKCS)
    add_subdirectory(plugins/logicalaccess/plugins/pkcs)
endif ()

if (NOT BUILD_TESTING STREQUAL OFF)
    enable_testing()
    add_subdirectory(tests)
endif ()

add_subdirectory(samples/basic)

if (NOT DEFINED SYSCONF_INSTALL_DIR)
    set(SYSCONF_INSTALL_DIR res)
endif ()
install(FILES liblogicalaccess.config DESTINATION ${SYSCONF_INSTALL_DIR})

# This perform the "real installation" of header files.
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/logicalaccess DESTINATION include/)
