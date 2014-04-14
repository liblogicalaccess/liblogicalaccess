cmake_minimum_required(VERSION 2.6)
project(ok5553readers)

if (WIN32)
	SET(VERSION_PRODUCTNAME_VALUE "ok5553readers")
	add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif()

set(INCLUDE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/mifare
		${CMAKE_CURRENT_SOURCE_DIR}/../../pluginsreaderproviders/iso7816
		${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/mifareultralight
		${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/desfire
		${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/twic
		${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/iso7816
		${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/samav2)

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
set(SOURCE_LIB ${SRCS} ${source_readercardadapters} ${source_commands})

add_library(
	ok5553readers
	SHARED
	${SOURCE_LIB}
	${INCLUDE_LIB}
	${resourcerc}
  )

SET_TARGET_PROPERTIES(ok5553readers PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})
  
target_link_libraries(
	ok5553readers
	mifarecards
	mifareultralightcards
	desfirecards
	twiccards
	iso7816readers
	iso7816cards
	${LIBLOGICALACCESS_LIBRARY}
)

install ( FILES ${include} DESTINATION include/logicalaccess/plugins/readers/ok5553 )
install ( FILES ${include_readercardadapters} DESTINATION include/logicalaccess/plugins/readers/ok5553/readercardadapters )
install ( FILES ${include_commands} DESTINATION include/logicalaccess/plugins/readers/ok5553/commands )
install ( TARGETS ok5553readers RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX} )
