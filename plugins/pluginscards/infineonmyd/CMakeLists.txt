cmake_minimum_required(VERSION 2.6)
project(infineonmydcards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	infineonmydcards
	SHARED

	${SRCS}
	${INCLUDE}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

SET_TARGET_PROPERTIES(infineonmydcards PROPERTIES VERSION 1.70)

target_link_libraries(
	infineonmydcards
	iso15693cards
	${LINK_OPT}
)