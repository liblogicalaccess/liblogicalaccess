cmake_minimum_required(VERSION 2.6)
project(mifarepluscards)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

set(INCLUDE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR})
			
INCLUDE_DIRECTORIES(${INCLUDE_FOLDER})


file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE include_mplus *.hpp)

add_library(
	mifarepluscards
	SHARED

	${SRCS}
	${include_core}
	${include_mplus}
  )

SET_TARGET_PROPERTIES(mifarepluscards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	mifarepluscards
	logicalaccess
	mifarecards
	desfirecards
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/mifareplus )
install ( TARGETS mifarepluscards LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
