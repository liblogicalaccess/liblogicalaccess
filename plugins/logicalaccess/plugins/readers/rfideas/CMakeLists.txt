cmake_minimum_required(VERSION 3.1)
project(rfideasreaders)

if (MSVC)
    SET(VERSION_PRODUCTNAME_VALUE "rfideasreaders")
    add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif ()

set(HEADER_RELATIVE_INSTALL_DIR "include/logicalaccess/plugins/readers/rfideas")

file(GLOB SRCS *.cpp)
file(GLOB include *.hpp)

set(INCLUDE_LIB ${include} ${include_core})
set(SOURCE_LIB ${SRCS})

add_library(rfideasreaders
        SHARED
        ${SOURCE_LIB}
        ${INCLUDE_LIB}
        ${LLA_RESOURCE_RC})

SET_TARGET_PROPERTIES(rfideasreaders PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(rfideasreaders
        logicalaccess
        CONAN_PKG::rfideas
        )
LLA_GENERATE_EXPORTS(LLA_READERS_RFIDEAS)

install(FILES ${include} DESTINATION ${HEADER_RELATIVE_INSTALL_DIR})
install(TARGETS rfideasreaders RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX})
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif ()
