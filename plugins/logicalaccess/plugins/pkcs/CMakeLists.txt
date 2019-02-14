cmake_minimum_required(VERSION 3.1)
project(pkcscryptounified)

if (MSVC)
    SET(VERSION_PRODUCTNAME_VALUE "pkcscryptounified")
    add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif ()

set(SRCS libraryentry.cpp)

add_library(
        pkcscryptounified
        SHARED
        ${SRCS}
        ${LLA_RESOURCE_RC}
)

SET_TARGET_PROPERTIES(pkcscryptounified PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
        pkcscryptounified
        CONAN_PKG::cppkcs11
        logicalaccess
)

install(FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/pkcs)
install(TARGETS pkcscryptounified RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX})
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif ()
