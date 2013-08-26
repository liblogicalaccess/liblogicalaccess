cmake_minimum_required(VERSION 2.6)
project(generictagcards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	generictagcards
	SHARED

	${SRCS}
	${INCLUDE}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

SET_TARGET_PROPERTIES(generictagcards PROPERTIES VERSION 1.67)

target_link_libraries(
	generictagcards
	${LINK_OPT}
)