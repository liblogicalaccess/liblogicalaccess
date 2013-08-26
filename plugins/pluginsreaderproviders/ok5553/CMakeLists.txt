cmake_minimum_required(VERSION 2.6)
project(ok5553readers)

if (NOT TARGET_ARCH)
	set(TARGET_ARCH "Win32")
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

set (LINK_OPT)
if (UNIX)
LINK_DIRECTORIES(${LINK_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/mifare/bin
				${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/mifareultralight/bin
				${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/desfire/bin
				${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/twic/bin
				${CMAKE_CURRENT_SOURCE_DIR}/../../pluginsreaderproviders/iso7816/bin
				${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/iso7816/bin
				${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/samav2/bin)
set(LINK_OPT winscard liblogicalaccess)
else()
LINK_DIRECTORIES(${LINK_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/mifare/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
				${CMAKE_CURRENT_SOURCE_DIR}/../../pluginsreaderproviders/iso7816/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
				${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/mifareultralight/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
				${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/desfire/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
				${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/twic/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
				${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/iso7816/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
				${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/samav2/bin/${TARGET_ARCH}/${VC_FOLDER_NAME})
endif()


add_library(
	ok5553readers
	SHARED
	
	${SOURCE_LIB}
	${INCLUDE_LIB}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess pthread boost_system boost_filesystem)
endif()

SET_TARGET_PROPERTIES(ok5553readers PROPERTIES VERSION 1.67)
  
target_link_libraries(
	ok5553readers
	mifarecards
	mifareultralightcards
	desfirecards
	twiccards
	iso7816readers
	iso7816cards
	${LINK_OPT}
)
