cmake_minimum_required(VERSION 3.5)
project(keyboardreaders)

if (MSVC)
	SET(VERSION_PRODUCTNAME_VALUE "keyboardreaders")
	add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif()

file(GLOB SRCS *.cpp)
file(GLOB include *.h)
file(GLOB includepp *.hpp)

file(GLOB include_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.hpp)
SOURCE_GROUP("Header Files\\readercardadapters" FILES ${include_readercardadapters})

file(GLOB source_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.cpp)
SOURCE_GROUP("Source Files\\readercardadapters" FILES ${source_readercardadapters})

set(INCLUDE_LIB ${include} ${includepp} ${include_core} ${include_readercardadapters})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters})


add_library(keyboardreaders
	SHARED	
	${SOURCE_LIB}
	${INCLUDE_LIB}
	${LLA_RESOURCE_RC})

SET_TARGET_PROPERTIES(keyboardreaders PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})
target_link_libraries(keyboardreaders
	logicalaccess
	UserEnv)
set(HEADER_RELATIVE_INSTALL_DIR "include/logicalaccess/plugins/cards/keyboard")
LLA_GENERATE_EXPORTS(LLA_READERS_PRIVATE_KEYBOARD)

install ( FILES ${include} ${includepp} DESTINATION include/logicalaccess/plugins/readers/keyboard )
install ( FILES ${include_readercardadapters} DESTINATION include/logicalaccess/plugins/readers/keyboard/readercardadapters )
install ( TARGETS keyboardreaders RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX} )
if (MSVC)
	install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif()


if (MSVC)
    add_subdirectory(islogkbdhooklib)
    add_subdirectory(islogkbdhook)
endif()
