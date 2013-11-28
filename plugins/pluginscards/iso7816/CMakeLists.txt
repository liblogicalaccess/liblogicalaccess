cmake_minimum_required(VERSION 2.6)
project(iso7816cards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	iso7816cards
	SHARED

	${SRCS}
	${INCLUDE}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

SET_TARGET_PROPERTIES(iso7816cards PROPERTIES VERSION 1.70)

target_link_libraries(
	iso7816cards
	${LINK_OPT}
)