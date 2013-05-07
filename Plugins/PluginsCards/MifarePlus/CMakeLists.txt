cmake_minimum_required(VERSION 2.6)
project(MifarePlusCards)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

set(INCLUDE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR})
			
if (UNIX)
	set(INCLUDE_FOLDER ${INCLUDE_FOLDER} /usr/include/PCSC)
endif()

INCLUDE_DIRECTORIES(${INCLUDE_FOLDER})


file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE include_mplus *.h)

add_library(
	MifarePlusCards
	SHARED

	${SRCS}
	${include_core}
	${include_mplus}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

SET_TARGET_PROPERTIES(MifarePlusCards PROPERTIES VERSION 2.0.3)

target_link_libraries(
	MifarePlusCards
	${LINK_OPT}
	MifareCards
	DESFireCards
)