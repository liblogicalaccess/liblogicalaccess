cmake_minimum_required(VERSION 2.6)
project(rplethreaders)

if (NOT TARGET_ARCH)
	set(TARGET_ARCH "Win32")
endif()

set(INCLUDE_FOLDER 	${CMAKE_CURRENT_SOURCE_DIR}/../../pluginsreaderproviders/iso7816
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/desfire
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/iso7816
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/samav2)

INCLUDE_DIRECTORIES(${INCLUDE_FOLDER})

file(GLOB SRCS *.cpp)
file(GLOB include *.hpp)

file(GLOB include_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.hpp)
SOURCE_GROUP("Header Files\\readercardadapters" FILES ${include_readercardadapters})

file(GLOB source_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.cpp)
SOURCE_GROUP("Source Files\\readercardadapters" FILES ${source_readercardadapters})

set(INCLUDE_LIB ${include} ${include_core} ${include_readercardadapters})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters})

set (LINK_OPT)
if (UNIX)
LINK_DIRECTORIES(${LINK_DIRECTORIES}	${CMAKE_CURRENT_SOURCE_DIR}/../../pluginsreaderproviders/iso7816/bin
										${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/desfire/bin
										${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/iso7816/bin
										${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/samav2/bin
										${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/twic/bin)
else()
LINK_DIRECTORIES(${LINK_DIRECTORIES}	${CMAKE_CURRENT_SOURCE_DIR}/../../pluginsreaderproviders/iso7816/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
										${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/desfire/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
										${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/iso7816/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
										${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/samav2/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}
										${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/twic/bin/${TARGET_ARCH}/${VC_FOLDER_NAME})
endif()

add_library(
	rplethreaders
	SHARED
	
	${SOURCE_LIB}
	${INCLUDE_LIB}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess pthread boost_system boost_filesystem)
endif()

SET_TARGET_PROPERTIES(rplethreaders PROPERTIES VERSION 1.70)
  
target_link_libraries(
	rplethreaders
	desfirecards
	iso7816readers
	iso7816cards
	${LINK_OPT}
)
