cmake_minimum_required(VERSION 2.6)
project(legicprimecards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	legicprimecards
	SHARED

	${SRCS}
	${INCLUDE}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

SET_TARGET_PROPERTIES(legicprimecards PROPERTIES VERSION 1.65)

target_link_libraries(
	legicprimecards
	${LINK_OPT}
)