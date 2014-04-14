cmake_minimum_required(VERSION 2.6)
project(iso7816readers)

if (WIN32)
	SET(VERSION_PRODUCTNAME_VALUE "iso7816readers")
	add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif()

set(INCLUDE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/desfire
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/twic
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/samav2
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/iso7816)
INCLUDE_DIRECTORIES(${INCLUDE_FOLDER})

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

set(INCLUDE_LIB ${include} ${include_core} ${include_readercardadapters} ${include_commands})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters} ${source_readercardadapters} ${source_commands})

add_library(
	iso7816readers
	SHARED
	${SOURCE_LIB}
	${INCLUDE_LIB}
	${resourcerc}
  )
  
SET_TARGET_PROPERTIES(iso7816readers PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})
  
target_link_libraries(
	iso7816readers
	desfirecards
	logicalaccess-cryptolib
	iso7816cards
	samav2cards
	${LIBLOGICALACCESS_LIBRARY}
	twiccards
)

install ( FILES ${include} DESTINATION include/logicalaccess/plugins/readers/iso7816 )
install ( FILES ${include_readercardadapters} DESTINATION include/logicalaccess/plugins/readers/iso7816/readercardadapters )
install ( FILES ${include_commands} DESTINATION include/logicalaccess/plugins/readers/iso7816/commands )
install ( TARGETS iso7816readers RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX} )