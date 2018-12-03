cmake_minimum_required(VERSION 3.1)
project(icode2cards)

if (MSVC)
    SET(VERSION_PRODUCTNAME_VALUE "icode2cards")
    add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif ()

set(HEADER_RELATIVE_INSTALL_DIR "include/logicalaccess/plugins/cards/icode2")

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(icode2cards
        SHARED
        ${SRCS}
        ${INCLUDE}
        ${LLA_RESOURCE_RC})

SET_TARGET_PROPERTIES(icode2cards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(icode2cards
        iso15693cards
        logicalaccess)
LLA_GENERATE_EXPORTS(LLA_CARDS_ICODE2)

install(FILES ${INCLUDE} DESTINATION ${HEADER_RELATIVE_INSTALL_DIR})
install(TARGETS icode2cards RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX})
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif ()
