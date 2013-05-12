cmake_minimum_required(VERSION 2.6)
project(GenericTagCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	GenericTagCards
	SHARED

	${SRCS}
	${INCLUDE}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

SET_TARGET_PROPERTIES(GenericTagCards PROPERTIES VERSION 2.0.3)

target_link_libraries(
	GenericTagCards
	${LINK_OPT}
)