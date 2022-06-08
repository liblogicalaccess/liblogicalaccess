cmake_minimum_required(VERSION 3.1)
project(yubikeycards)

if (MSVC)
    SET(VERSION_PRODUCTNAME_VALUE "yubikeycards")
    add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif ()

set(HEADER_RELATIVE_INSTALL_DIR "include/logicalaccess/plugins/cards/yubikey")

set(SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/yubikeychip.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/yubikeychallengecardservice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libraryentry.cpp)

set(HEADER_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/yubikeychip.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/yubikeychallengecardservice.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/yubikeycommands.hpp)

add_library(yubikeycards SHARED
        ${SOURCE_FILES} ${HEADER_FILES}
        ${LLA_RESOURCE_RC})

SET_TARGET_PROPERTIES(yubikeycards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(yubikeycards
        iso7816cards
        logicalaccess)
LLA_GENERATE_EXPORTS(LLA_CARDS_YUBIKEY)

install(FILES ${HEADER_FILES} DESTINATION ${HEADER_RELATIVE_INSTALL_DIR})
install(TARGETS yubikeycards RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX})
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif ()
