cmake_minimum_required(VERSION 2.6)
project(MifareCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	MifareCards
	SHARED

	${SRCS}
	${INCLUDE}
  )

SET_TARGET_PROPERTIES(liblogicalaccess PROPERTIES VERSION 2.0.3)

set(LINK_OPT)
if (UNIX)
  set(LINK_OPT crypto)
endif()

target_link_libraries(
	MifareCards
	liblogicalaccess
	${LINK_OPT}
)