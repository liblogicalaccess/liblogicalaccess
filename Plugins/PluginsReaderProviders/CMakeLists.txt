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
  set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra")
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
			"${CMAKE_CURRENT_SOURCE_DIR}/../Cryptolib/include")
  LINK_DIRECTORIES(${LINK_DIRECTORIES} "${CMAKE_CURRENT_SOURCE_DIR}/../../packages/lib"
					"${CMAKE_CURRENT_SOURCE_DIR}/../../bin"
					"${CMAKE_CURRENT_SOURCE_DIR}/../Cryptolib/bin")
else()
  INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../../packages/include"
			"${CMAKE_CURRENT_SOURCE_DIR}/../../include"
			"${CMAKE_CURRENT_SOURCE_DIR}/../Cryptolib/include")
  LINK_DIRECTORIES(${LINK_DIRECTORIES} "${CMAKE_CURRENT_SOURCE_DIR}/../../packages/lib/${TARGET_ARCH}"
					"${CMAKE_CURRENT_SOURCE_DIR}/../Cryptolib/bin/${TARGET_ARCH}/${VC_FOLDER_NAME}"
					"${CMAKE_CURRENT_SOURCE_DIR}/../../bin/${TARGET_ARCH}/${VC_FOLDER_NAME}")
endif()


add_subdirectory(ISO7816)
add_subdirectory(A3MLGM5600)
add_subdirectory(Admitto)
add_subdirectory(AxessTMC13)
add_subdirectory(AxessTMCLegic)
add_subdirectory(Deister)
add_subdirectory(Elatec)
add_subdirectory(Gunnebo)
add_subdirectory(IdOnDemand)
add_subdirectory(PCSC)
add_subdirectory(Promag)
add_subdirectory(Rpleth)
add_subdirectory(SCIEL)
add_subdirectory(SmartID)
add_subdirectory(STidSTR)
if(WIN32)
add_subdirectory(Keyboard)
add_subdirectory(RFIDeas)
endif()
