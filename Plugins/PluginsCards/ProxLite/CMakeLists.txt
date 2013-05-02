cmake_minimum_required(VERSION 2.6)
project(ProxLiteCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	ProxLiteCards
	SHARED

	${SRCS}
	${INCLUDE}
  )

target_link_libraries(
	ProxLiteCards
	liblogicalaccess
)