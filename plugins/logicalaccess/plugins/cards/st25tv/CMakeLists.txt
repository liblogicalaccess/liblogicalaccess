cmake_minimum_required(VERSION 3.1)
project(st25tvcards)

if (MSVC)
    SET(VERSION_PRODUCTNAME_VALUE "st25tvcards")
    add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif ()

set(HEADER_RELATIVE_INSTALL_DIR "include/logicalaccess/plugins/cards/st25tv")

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(st25tvcards
        SHARED
        ${SRCS}
        ${INCLUDE}
        ${LLA_RESOURCE_RC})

SET_TARGET_PROPERTIES(st25tvcards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(st25tvcards
        logicalaccess)
LLA_GENERATE_EXPORTS(LLA_CARDS_ST25TV)

install(FILES ${INCLUDE} DESTINATION ${HEADER_RELATIVE_INSTALL_DIR})
install(TARGETS st25tvcards RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX})
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif ()
