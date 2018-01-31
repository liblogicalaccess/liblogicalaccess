cmake_minimum_required(VERSION 3.1)
project(remotecryptoiksunified)

if (WIN32)
    SET(VERSION_PRODUCTNAME_VALUE "remotecryptoiksunified")
    add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif ()

file(GLOB_RECURSE SRCS *.cpp *.cc)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
        remotecryptoiksunified
        SHARED
        ${SRCS}
        ${INCLUDE}
        ${resourcerc}
)

target_compile_definitions(remotecryptoiksunified PRIVATE _WIN32_WINNT=0x600)

SET_TARGET_PROPERTIES(remotecryptoiksunified PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
    target_link_libraries(
            remotecryptoiksunified
            grpc grpc++ libprotobufd zlibd gpr
            ${LIBLOGICALACCESS_LIBRARY}
    )
elseif (CMAKE_BUILD_TYPE STREQUAL "Release" AND MSVC)
    target_link_libraries(
            remotecryptoiksunified
            grpc grpc++ libprotobuf zlib gpr
            ${LIBLOGICALACCESS_LIBRARY}
    )
else ()
    target_link_libraries(
            remotecryptoiksunified
            grpc grpc++ protobuf gpr
            ${LIBLOGICALACCESS_LIBRARY}
    )
endif ()

install(FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/iks)
install(TARGETS remotecryptoiksunified RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX})
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif ()
