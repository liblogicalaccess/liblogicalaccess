cmake_minimum_required(VERSION 2.6)
project(smartframecards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	smartframecards
	SHARED

	${SRCS}
	${INCLUDE}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

SET_TARGET_PROPERTIES(smartframecards PROPERTIES VERSION 1.70)

target_link_libraries(
	smartframecards
	${LINK_OPT}
)