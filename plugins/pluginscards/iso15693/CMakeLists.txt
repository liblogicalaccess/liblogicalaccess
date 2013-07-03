cmake_minimum_required(VERSION 2.6)
project(iso15693cards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	iso15693cards
	SHARED

	${SRCS}
	${INCLUDE}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

SET_TARGET_PROPERTIES(iso15693cards PROPERTIES VERSION 1.65)

target_link_libraries(
	iso15693cards
	${LINK_OPT}
)