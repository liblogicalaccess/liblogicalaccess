cmake_minimum_required(VERSION 2.6)
project(MifareCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	MifareCards
	SHARED

	${SRCS}
	${INCLUDE}
  )

target_link_libraries(
	MifareCards
	liblogicalaccess
)