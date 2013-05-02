cmake_minimum_required(VERSION 2.6)
project(PCSCReaders)

if (NOT TARGET_ARCH)
	set(TARGET_ARCH "Win32")
endif()

set(INCLUDE_FOLDER 	${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsReaderProviders/ISO7816
					${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/DESFire
					${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/Mifare
					${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/Prox
					${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/Twic
					${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/TagIt
					${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/ISO15693
					${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/MifareUltralight
					${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/MifarePlus
					${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/ISO7816
					/usr/include/PCSC)
if (UNIX)
  set(INCLUDE_FOLDER ${INCLUDE_FOLDER} "/usr/local/include/PCSC/")
endif()

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

file(GLOB include_readers ${CMAKE_CURRENT_SOURCE_DIR}/Readers/*.h)
SOURCE_GROUP("Header Files\\Readers" FILES ${include_readers})

file(GLOB source_readers ${CMAKE_CURRENT_SOURCE_DIR}/Readers/*.cpp)
SOURCE_GROUP("Source Files\\Readers" FILES ${source_readers})


set(INCLUDE_LIB ${include} ${include_core} ${include_readercardadapters} ${include_readers} ${include_commands})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters} ${source_readers} ${source_commands})

					
set (LINK_OPT)
if (UNIX)
LINK_DIRECTORIES(${LINK_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsReaderProviders/ISO7816/bin
				${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/DESFire/bin
				${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/MifarePlus/bin
				${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/Mifare/bin
				${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/MifareUltralight/bin
				${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/ISO7816/bin
				${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/ISO15693/bin
				${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/Twic/bin
				${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/Prox/bin)
else()
LINK_DIRECTORIES(${LINK_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsReaderProviders/ISO7816/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
				${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/DESFire/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
				${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/MifarePlus/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
				${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/Mifare/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
				${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/MifareUltralight/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
				${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/ISO7816/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
				${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/ISO15693/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
				${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/Twic/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
				${CMAKE_CURRENT_SOURCE_DIR}/../../PluginsCards/Prox/bin/${TARGET_ARCH}/${VC_FOLDER_NAME})
		
set(LINK_OPT winscard)
endif()


add_library(
	PCSCReaders
	SHARED
	
	${SOURCE_LIB}
	${INCLUDE_LIB}
  )
  
target_link_libraries(
	PCSCReaders
	liblogicalaccess
	DESFireCards
	MifareCards
	MifarePlusCards
	MifareUltralightCards
	ISO7816Readers
	ISO7816Cards
	ISO15693Cards
	ProxCards
	${LINK_OPT}
)
