cmake_minimum_required(VERSION 2.6)
project(stidstrreaders)

if (NOT TARGET_ARCH)
	set(TARGET_ARCH "Win32")
endif()

set(INCLUDE_FOLDER 	${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/desfire
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/mifare)
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

if (WIN32)
LINK_DIRECTORIES(${LINK_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/mifare/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
										${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/desfire/bin/${TARGET_ARCH}/${VC_FOLDER_NAME})
else()
LINK_DIRECTORIES(${LINK_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/mifare/bin
										${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/desfire/bin)
endif()

add_library(
	stidstrreaders
	SHARED
	
	${SOURCE_LIB}
	${INCLUDE_LIB}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess libeay32)
else()
  set(LINK_OPT logicalaccess  crypto boost_filesystem boost_system)
endif()

SET_TARGET_PROPERTIES(stidstrreaders PROPERTIES VERSION 1.70)  

target_link_libraries(
	stidstrreaders
	logicalaccess-cryptolib
	mifarecards
	desfirecards
	${LINK_OPT}
)