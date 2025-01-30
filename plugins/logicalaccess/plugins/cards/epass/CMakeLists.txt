cmake_minimum_required(VERSION 3.10)
project(epasscards)

if (MSVC)
    SET(VERSION_PRODUCTNAME_VALUE "epasscards")
    add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif ()

set(HEADER_RELATIVE_INSTALL_DIR "include/logicalaccess/plugins/cards/epass")

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
        epasscards
        SHARED

        ${SRCS}
        ${INCLUDE}
        ${LLA_RESOURCE_RC}
)

SET_TARGET_PROPERTIES(epasscards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(epasscards
        iso7816cards
        logicalaccess)
LLA_GENERATE_EXPORTS(LLA_CARDS_EPASS)

install(FILES ${INCLUDE} DESTINATION ${HEADER_RELATIVE_INSTALL_DIR})
install(TARGETS epasscards RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX})
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif ()
