cmake_minimum_required(VERSION 2.6)
project(IndalaCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	IndalaCards
	SHARED

	${SRCS}
	${INCLUDE}
  )

target_link_libraries(
	IndalaCards
	liblogicalaccess
)