cmake_minimum_required(VERSION 2.6)
project(SmartFrameCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	SmartFrameCards
	SHARED

	${SRCS}
	${INCLUDE}
  )

target_link_libraries(
	SmartFrameCards
	liblogicalaccess
)