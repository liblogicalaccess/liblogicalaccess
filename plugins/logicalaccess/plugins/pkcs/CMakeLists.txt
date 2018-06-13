cmake_minimum_required(VERSION 3.1)
project(pkcscryptounified)

add_subdirectory(cppkcs11/cppkcs11)

if (WIN32)
    SET(VERSION_PRODUCTNAME_VALUE "pkcscryptounified")
    add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif ()

file(GLOB_RECURSE SRCS *.cpp *.cc)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
        pkcscryptounified
        SHARED
        ${SRCS}
        ${INCLUDE}
        ${resourcerc}
)

SET_TARGET_PROPERTIES(pkcscryptounified PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
    target_link_libraries(
            pkcscryptounified
            cppkcs11
            ${LIBLOGICALACCESS_LIBRARY}
    )
elseif (CMAKE_BUILD_TYPE STREQUAL "Release" AND MSVC)
    target_link_libraries(
            pkcscryptounified
            cppkcs11
            ${LIBLOGICALACCESS_LIBRARY}
    )
else ()
    target_link_libraries(
            pkcscryptounified
            cppkcs11
            ${LIBLOGICALACCESS_LIBRARY}
    )
endif ()

install(FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/pkcs)
install(TARGETS pkcscryptounified RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX})
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif ()
