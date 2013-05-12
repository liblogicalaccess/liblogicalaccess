cmake_minimum_required(VERSION 2.6)
project(MifareCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	MifareCards
	SHARED

	${SRCS}
	${INCLUDE}
  )

SET_TARGET_PROPERTIES(MifareCards PROPERTIES VERSION 2.0.3)

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess crypto)
endif()

target_link_libraries(
	MifareCards
	${LINK_OPT}
)