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

SET_TARGET_PROPERTIES(ProxCards PROPERTIES VERSION 2.0.3)

target_link_libraries(
	ProxCards
	liblogicalaccess
)