cmake_minimum_required(VERSION 2.6)
project(em4102cards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	em4102cards
	SHARED

	${SRCS}
	${INCLUDE}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

SET_TARGET_PROPERTIES(em4102cards PROPERTIES VERSION 1.70)

target_link_libraries(
	em4102cards
	${LINK_OPT}
)