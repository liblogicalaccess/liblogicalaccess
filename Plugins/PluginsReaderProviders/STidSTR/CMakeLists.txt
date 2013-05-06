cmake_minimum_required(VERSION 2.6)
project(STidSTRReaders)

if (NOT TARGET_ARCH)
	set(TARGET_ARCH "Win32")
endif()

set(INCLUDE_FOLDER 	${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/DESFire
					${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/Mifare)
INCLUDE_DIRECTORIES(${INCLUDE_FOLDER})


file(GLOB SRCS *.cpp)
file(GLOB include *.h)

file(GLOB include_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/ReaderCardAdapters/*.h)
SOURCE_GROUP("Header Files\\ReaderCardAdapters" FILES ${include_readercardadapters})

file(GLOB source_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/ReaderCardAdapters/*.cpp)
SOURCE_GROUP("Source Files\\ReaderCardAdapters" FILES ${source_readercardadapters})

file(GLOB include_commands ${CMAKE_CURRENT_SOURCE_DIR}/Commands/*.h)
SOURCE_GROUP("Header Files\\Commands" FILES ${include_commands})

file(GLOB source_commands ${CMAKE_CURRENT_SOURCE_DIR}/Commands/*.cpp)
SOURCE_GROUP("Source Files\\Commands" FILES ${source_commands})


set(INCLUDE_LIB ${include} ${include_core} ${include_readercardadapters} ${include_commands})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters} ${source_commands})

if (WIN32)
LINK_DIRECTORIES(${LINK_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/Mifare/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
										${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/DESFire/bin/${TARGET_ARCH}/${VC_FOLDER_NAME})
else()
LINK_DIRECTORIES(${LINK_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/Mifare/bin
										${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/DESFire/bin)
endif()

add_library(
	STidSTRReaders
	SHARED
	
	${SOURCE_LIB}
	${INCLUDE_LIB}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess libeay32)
else()
  set(LINK_OPT logicalaccess  crypto boost_filesystem boost_system)
endif()
  
target_link_libraries(
	STidSTRReaders
	logicalaccess-cryptolib
	MifareCards
	DESFireCards
	${OPT_LIBS}
)