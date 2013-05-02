cmake_minimum_required(VERSION 2.6)
project(ProxCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	ProxCards
	SHARED

	${SRCS}
	${INCLUDE}
  )

target_link_libraries(
	ProxCards
	liblogicalaccess
)