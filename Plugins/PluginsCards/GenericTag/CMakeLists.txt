cmake_minimum_required(VERSION 2.6)
project(GenericTagCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	GenericTagCards
	SHARED

	${SRCS}
	${INCLUDE}
  )

SET_TARGET_PROPERTIES(GenericTagCards PROPERTIES VERSION 2.0.3)

target_link_libraries(
	GenericTagCards
	liblogicalaccess
)