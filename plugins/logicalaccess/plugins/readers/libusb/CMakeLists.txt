cmake_minimum_required(VERSION 3.1)
project(libusbreaders)

if (MSVC)
    SET(VERSION_PRODUCTNAME_VALUE "libusbreaders")
    add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif ()

set(HEADER_RELATIVE_INSTALL_DIR "include/logicalaccess/plugins/readers/libusb")

file(GLOB SRCS *.cpp)
file(GLOB include *.hpp)

file(GLOB include_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.hpp)
SOURCE_GROUP("Header Files\\readercardadapters" FILES ${include_readercardadapters})

file(GLOB source_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.cpp)
SOURCE_GROUP("Source Files\\readercardadapters" FILES ${source_readercardadapters})

set(INCLUDE_LIB ${include} ${include_core} ${include_readercardadapters})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters})

add_library(libusbreaders
        SHARED
        ${SOURCE_LIB}
        ${INCLUDE_LIB}
        ${LLA_RESOURCE_RC})

SET_TARGET_PROPERTIES(libusbreaders PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(libusbreaders
		yubikeycards
		iso7816readers
		logicalaccess
		CONAN_PKG::libusb
		CONAN_PKG::boost
		)

LLA_GENERATE_EXPORTS(LLA_READERS_LIBUSB)

install(FILES ${include} DESTINATION ${HEADER_RELATIVE_INSTALL_DIR})
install(FILES ${include_readercardadapters} DESTINATION ${HEADER_RELATIVE_INSTALL_DIR}/readercardadapters)
install(TARGETS libusbreaders RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX})
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif ()
