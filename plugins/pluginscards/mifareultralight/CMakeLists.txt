cmake_minimum_required(VERSION 2.6)
project(mifareultralightcards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	mifareultralightcards
	SHARED

	${SRCS}
	${INCLUDE}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

SET_TARGET_PROPERTIES(mifareultralightcards PROPERTIES VERSION 1.65)

target_link_libraries(
	mifareultralightcards
	${LINK_OPT}
)