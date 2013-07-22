cmake_minimum_required(VERSION 2.6)

project(basictest)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

include_directories("../../include" "../../plugins/pluginsreaderproviders/iso7816" "../../plugins/pluginsreaderproviders/pcsc" "../../plugins/pluginscards/samav2" "../../plugins/pluginscards/desfire")

if(UNIX)
  set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra")
  add_definitions(-DUNIX)
endif()

SET(SOURCE basic.cpp)

add_executable(
    basic
    ${SOURCE}
)

if(UNIX)
	set(${LINK_DIRECTORY} "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/${TARGET_ARCH}/${VC_FOLDER_NAME}" "../../plugins/pluginscards/desfire")
  link_directories(${LINK_DIRECTORY})
else()
	set(${LINK_DIRECTORY} "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/${TARGET_ARCH}/${VC_FOLDER_NAME}" 				"${CMAKE_CURRENT_SOURCE_DIR}/../../plugins/pluginscards/desfire/${TARGET_ARCH}/${VC_FOLDER_NAME}"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../plugins/pluginsreaderproviders/pcsc/${TARGET_ARCH}/${VC_FOLDER_NAME}")
  link_directories(${LINK_DIRECTORY})
endif()

remove_definitions(-DLIBLOGICALACCESS_EXPORTS -DLIBLOGICALACCESS_EXPORTS_DLL)

set(LIB_OPT)
if(UNIX)

set(LIB_ENDNAME)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(LIB_ENDNAME ".debug")
endif()

  set(LIB_OPT
    ssl
    boost_filesystem
    boost_date_time
    logicalaccess-cryptolib
    )
endif()

target_link_libraries(
        basic
        liblogicalaccess
		pcscreaders
		iso7816readers
		desfirecards
	${LIB_OPT}
)
