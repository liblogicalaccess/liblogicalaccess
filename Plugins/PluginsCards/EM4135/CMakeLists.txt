cmake_minimum_required(VERSION 2.6)
project(EM4135Cards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	EM4135Cards
	SHARED

	${SRCS}
	${INCLUDE}
  )

target_link_libraries(
	EM4135Cards
	liblogicalaccess
)