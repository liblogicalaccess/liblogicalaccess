file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE HDRS *.h)
file(GLOB_RECURSE HDPPRS *.hpp)

set(INCLUDE ${HDRS} ${HDPPRS})

find_package(OpenSSL)

add_library(logicalaccess-cryptolib
        SHARED
        ${SRCS}
        ${INCLUDE})

if (MSVC)
    target_compile_options(logicalaccess-cryptolib PUBLIC "/wd4251")
endif()
target_link_libraries(logicalaccess-cryptolib PUBLIC llacommon OpenSSL::Crypto)

# Export header generation and installation
generate_export_header(logicalaccess-cryptolib
        EXPORT_MACRO_NAME LLA_CRYPTO_API
        EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/lla_crypto_api.hpp")
target_compile_definitions(logicalaccess-cryptolib PRIVATE logicalaccess_cryptolib_EXPORTS=1)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/lla_crypto_api.hpp" DESTINATION include/logicalaccess/plugins/crypto)

install(FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/crypto)
install(TARGETS logicalaccess-cryptolib RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX})
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif ()
