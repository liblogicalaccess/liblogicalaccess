cmake_minimum_required(VERSION 3.10)
project(cps3cards)

if (MSVC)
    SET(VERSION_PRODUCTNAME_VALUE "cps3cards")
    add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif ()

set(HEADER_RELATIVE_INSTALL_DIR "include/logicalaccess/plugins/cards/cps3")

set(SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/cps3chip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cps3commands.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cps3location.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cps3storagecardservice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libraryentry.cpp)

set(HEADER_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/cps3chip.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cps3commands.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cps3location.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cps3storagecardservice.hpp)

add_library(cps3cards SHARED
        ${SOURCE_FILES} ${HEADER_FILES}
        ${LLA_RESOURCE_RC})

SET_TARGET_PROPERTIES(cps3cards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(cps3cards
        iso7816cards
        logicalaccess)
LLA_GENERATE_EXPORTS(LLA_CARDS_CPS3)

install(FILES ${HEADER_FILES} DESTINATION ${HEADER_RELATIVE_INSTALL_DIR})
install(TARGETS cps3cards RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX})
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif ()
