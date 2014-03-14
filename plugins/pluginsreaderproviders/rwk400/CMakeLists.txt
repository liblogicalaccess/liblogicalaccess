cmake_minimum_required(VERSION 2.6)
project(rwk400readers)

if (NOT TARGET_ARCH)
	set(TARGET_ARCH "Win32")
endif()

set(INCLUDE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/desfire
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
set(SOURCE_LIB ${SRCS} ${source_readercardadapters} ${source_commands})


add_library(
	rwk400readers
	SHARED
	${SOURCE_LIB}
  )

SET_TARGET_PROPERTIES(deisterreaders PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})
  
target_link_libraries(
	rwk400readers
	desfirecards
	iso7816cards
	logicalaccess
)


install ( FILES ${include} DESTINATION include/logicalaccess/plugins/readers/rwk400 )
install ( FILES ${include_readercardadapters} DESTINATION include/logicalaccess/plugins/readers/rwk400/readercardadapters )
install ( FILES ${include_commands} DESTINATION include/logicalaccess/plugins/readers/rwk400/readercardadapters )
install ( TARGETS rwk400readers LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )

