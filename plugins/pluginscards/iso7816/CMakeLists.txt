cmake_minimum_required(VERSION 2.6)
project(ISO7816Cards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	ISO7816Cards
	SHARED

	${SRCS}
	${INCLUDE}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

SET_TARGET_PROPERTIES(ISO7816Cards PROPERTIES VERSION 2.0.3)

target_link_libraries(
	ISO7816Cards
	${LINK_OPT}
)