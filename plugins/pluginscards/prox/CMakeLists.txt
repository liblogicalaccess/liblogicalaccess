cmake_minimum_required(VERSION 2.6)
project(proxcards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	proxcards
	SHARED

	${SRCS}
	${INCLUDE}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

SET_TARGET_PROPERTIES(proxcards PROPERTIES VERSION 1.70)

target_link_libraries(
	proxcards
	${LINK_OPT}
)