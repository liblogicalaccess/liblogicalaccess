cmake_minimum_required(VERSION 3.1)

project(remotecryptoiksunified)
if (MSVC)
    SET(VERSION_PRODUCTNAME_VALUE "remotecryptoiksunified")
    add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif ()

file(GLOB_RECURSE SRCS *.cpp *.cc)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(remotecryptoiksunified
        SHARED
        ${SRCS}
        ${INCLUDE}
        ${LLA_RESOURCE_RC})

target_compile_definitions(remotecryptoiksunified PRIVATE _WIN32_WINNT=0x600)

SET_TARGET_PROPERTIES(remotecryptoiksunified PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(remotecryptoiksunified
        CONAN_PKG::grpc
        logicalaccess)
generate_export_header(remotecryptoiksunified
        EXPORT_MACRO_NAME LLA_IKS_API
        EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/lla_iks_api.hpp")
target_compile_definitions(remotecryptoiksunified PRIVATE remotecryptoiksunified_EXPORTS=1)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/lla_iks_api.hpp" DESTINATION include/logicalaccess/plugins/iks)

install(FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/iks)
install(TARGETS remotecryptoiksunified RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX})
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif ()
