cmake_minimum_required(VERSION 2.6)
project(indalacards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	indalacards
	SHARED

	${SRCS}
	${INCLUDE}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

SET_TARGET_PROPERTIES(indalacards PROPERTIES VERSION 1.67)

target_link_libraries(
	indalacards
	${LINK_OPT}
)