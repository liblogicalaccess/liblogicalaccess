cmake_minimum_required(VERSION 2.6)
project(mifarecards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	mifarecards
	SHARED

	${SRCS}
	${INCLUDE}
  )

SET_TARGET_PROPERTIES(mifarecards PROPERTIES VERSION 1.67)

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess crypto)
endif()

SET_TARGET_PROPERTIES(mifarecards PROPERTIES VERSION 1.67)

target_link_libraries(
	mifarecards
	${LINK_OPT}
)