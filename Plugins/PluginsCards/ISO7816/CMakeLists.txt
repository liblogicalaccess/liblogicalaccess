cmake_minimum_required(VERSION 2.6)
project(ISO7816Cards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	ISO7816Cards
	SHARED

	${SRCS}
	${INCLUDE}
  )

SET_TARGET_PROPERTIES(ISO7816Cards PROPERTIES VERSION 2.0.3)

target_link_libraries(
	ISO7816Cards
	liblogicalaccess
)