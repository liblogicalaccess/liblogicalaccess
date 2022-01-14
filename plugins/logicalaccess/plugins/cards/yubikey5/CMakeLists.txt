cmake_minimum_required(VERSION 3.1)
project(yubikey5cards)

if (MSVC)
    SET(VERSION_PRODUCTNAME_VALUE "yubikey5cards")
    add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif ()

set(HEADER_RELATIVE_INSTALL_DIR "include/logicalaccess/plugins/cards/yubikey5")

set(SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/yubikey5chip.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libraryentry.cpp)

set(HEADER_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/yubikey5chip.hpp)

add_library(yubikey5cards SHARED
        ${SOURCE_FILES} ${HEADER_FILES}
        ${LLA_RESOURCE_RC})

SET_TARGET_PROPERTIES(yubikey5cards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(yubikey5cards
        iso7816cards
        logicalaccess)
LLA_GENERATE_EXPORTS(LLA_CARDS_YUBIKEY5)

install(FILES ${HEADER_FILES} DESTINATION ${HEADER_RELATIVE_INSTALL_DIR})
install(TARGETS yubikey5cards RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX})
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif ()
