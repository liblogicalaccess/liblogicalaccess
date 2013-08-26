cmake_minimum_required(VERSION 2.6)

project(pluginsreaders-liblogicalaccess)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

if (NOT TARGET_ARCH)
	set(TARGET_ARCH "Win32")
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(-DDEBUG -D_DEBUG)
	set(VC_FOLDER_NAME "Debug")
else()
	set(VC_FOLDER_NAME "Release")
endif()

if(UNIX)
  set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -g")
  add_definitions(-DUNIX -DLOGICALACCESS_LOGS)
else()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  
  if (TARGET_ARCH STREQUAL "Win32")
    add_definitions(-DWIN32 -D_WIN32)
  else()
	add_definitions(-DWIN64 -D_WIN64)
  endif()
  add_definitions(-DLOGICALACCESS_LOGS -D_USRDLL -DS_WNT -D_CRT_SECURE_NO_WARNINGS -DLIBLOGICALACCESS_EXPORTS)
endif()

if(UNIX)
	SET(LIBRARY_OUTPUT_PATH ./bin)
else()
	SET(LIBRARY_OUTPUT_PATH ./bin/${TARGET_ARCH})
endif()

if (UNIX)
  INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../../packages/include"
			"${CMAKE_CURRENT_SOURCE_DIR}/../../include"
			"${CMAKE_CURRENT_SOURCE_DIR}/../cryptolib/include")
  LINK_DIRECTORIES(${LINK_DIRECTORIES} "${CMAKE_CURRENT_SOURCE_DIR}/../../packages/lib"
					"${CMAKE_CURRENT_SOURCE_DIR}/../../bin"
					"${CMAKE_CURRENT_SOURCE_DIR}/../cryptolib/bin")
else()
  INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../../packages/include"
			"${CMAKE_CURRENT_SOURCE_DIR}/../../include"
			"${CMAKE_CURRENT_SOURCE_DIR}/../cryptolib/include")
  LINK_DIRECTORIES(${LINK_DIRECTORIES} "${CMAKE_CURRENT_SOURCE_DIR}/../../packages/lib/${TARGET_ARCH}"
					"${CMAKE_CURRENT_SOURCE_DIR}/../cryptolib/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}"
					"${CMAKE_CURRENT_SOURCE_DIR}/../../bin/${TARGET_ARCH}/${VC_FOLDER_NAME}")
endif()


add_subdirectory(iso7816)
add_subdirectory(a3mlgm5600)
add_subdirectory(admitto)
add_subdirectory(axesstmc13)
add_subdirectory(axesstmclegic)
add_subdirectory(deister)
add_subdirectory(elatec)
add_subdirectory(gunnebo)
add_subdirectory(idondemand)
add_subdirectory(ok5553)
add_subdirectory(pcsc)
add_subdirectory(promag)
add_subdirectory(rpleth)
add_subdirectory(rwk400)
add_subdirectory(sciel)
add_subdirectory(smartid)
add_subdirectory(stidstr)
if(WIN32)
add_subdirectory(keyboard)
add_subdirectory(rfideas)
endif()
