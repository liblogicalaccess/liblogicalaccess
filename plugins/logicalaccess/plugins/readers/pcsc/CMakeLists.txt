cmake_minimum_required(VERSION 3.10)
project(pcscreaders)

if (MSVC)
    SET(VERSION_PRODUCTNAME_VALUE "pcscreaders")
    add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif ()

find_package(Boost)

if (UNIX)
    find_package(PCSCLite REQUIRED)
endif ()

set(HEADER_RELATIVE_INSTALL_DIR "include/logicalaccess/plugins/readers/pcsc")

file(GLOB SRCS *.cpp)
file(GLOB include *.hpp)

file(GLOB include_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.hpp)
SOURCE_GROUP("Header Files\\readercardadapters" FILES ${include_readercardadapters})

file(GLOB source_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.cpp)
SOURCE_GROUP("Source Files\\readercardadapters" FILES ${source_readercardadapters})

file(GLOB include_commands ${CMAKE_CURRENT_SOURCE_DIR}/commands/*.hpp)
SOURCE_GROUP("Header Files\\commands" FILES ${include_commands})

file(GLOB source_commands ${CMAKE_CURRENT_SOURCE_DIR}/commands/*.cpp)
SOURCE_GROUP("Source Files\\commands" FILES ${source_commands})

file(GLOB include_readers ${CMAKE_CURRENT_SOURCE_DIR}/readers/*.hpp)
SOURCE_GROUP("Header Files\\readers" FILES ${include_readers})

file(GLOB source_readers ${CMAKE_CURRENT_SOURCE_DIR}/readers/*.cpp)
SOURCE_GROUP("Source Files\\readers" FILES ${source_readers})

file(GLOB source_probes ${CMAKE_CURRENT_SOURCE_DIR}/readers/cardprobes/*.cpp)
SOURCE_GROUP("Source Files\\readers\\cardprobes" FILES ${source_probes})

file(GLOB include_probes ${CMAKE_CURRENT_SOURCE_DIR}/readers/cardprobes/*.hpp)
SOURCE_GROUP("Header Files\\readers\\cardprobes" FILES ${include_probes})

set(INCLUDE_LIB ${include} ${include_core} ${include_readercardadapters}
        ${include_readers} ${include_commands} ${include_probes})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters} ${source_readers}
        ${source_commands} ${source_probes})

add_library(pcscreaders
        SHARED
        ${SOURCE_LIB}
        ${INCLUDE_LIB}
        ${LLA_RESOURCE_RC})

SET_TARGET_PROPERTIES(pcscreaders PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

if (UNIX)
    target_link_libraries(pcscreaders
            desfirecards
            mifarecards
            mifarepluscards
            mifareultralightcards
            iso7816readers
            twiccards
            iso7816cards
            iso15693cards
            tagitcards
            proxcards
            felicacards
            epasscards
            topazcards
            generictagcards
            seoscards
			yubikeycards
            logicalaccess
            PCSCLite::PCSCLite
            Boost::boost
            )
else ()
    target_link_libraries(pcscreaders
            desfirecards
            mifarecards
            mifarepluscards
            mifareultralightcards
            iso7816readers
            twiccards
            iso7816cards
            iso15693cards
            tagitcards
            proxcards
            felicacards
            epasscards
            topazcards
            generictagcards
            seoscards
			yubikeycards
			winscard
            logicalaccess
            Boost::boost)
endif ()

LLA_GENERATE_EXPORTS(LLA_READERS_PCSC)

install(FILES ${include} DESTINATION ${HEADER_RELATIVE_INSTALL_DIR})
install(FILES ${include_readercardadapters} DESTINATION ${HEADER_RELATIVE_INSTALL_DIR}/readercardadapters)
install(FILES ${include_commands} DESTINATION ${HEADER_RELATIVE_INSTALL_DIR}/commands)
install(FILES ${include_readers} DESTINATION ${HEADER_RELATIVE_INSTALL_DIR}/readers)
install(TARGETS pcscreaders RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX})
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif ()
