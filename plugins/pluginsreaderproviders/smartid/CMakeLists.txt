cmake_minimum_required(VERSION 2.6)
project(SmartIDReaders)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

set(INCLUDE_FOLDER 	${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsReaderProviders/ISO7816
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

set(LINK_OPT)
if (WIN32)
LINK_DIRECTORIES(${LINK_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/Mifare/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
										${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/DESFire/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
										${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/ISO7816/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
										${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/Twic/bin/${TARGET_ARCH}/${VC_FOLDER_NAME})
set(LINK_OPT liblogicalaccess)
else()
LINK_DIRECTORIES(${LINK_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/Mifare/bin
										${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/DESFire/bin
										${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/ISO7816/bin
										${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/Twic/bin)
										
set(LINK_OPT boost_system boost_filesystem logicalaccess)
endif()
										
add_library(
	SmartIDReaders
	SHARED
	
	${SOURCE_LIB}
	${INCLUDE_LIB}
  )
  
target_link_libraries(
	SmartIDReaders
	ISO7816Readers
	MifareCards
	${LINK_OPT}
)