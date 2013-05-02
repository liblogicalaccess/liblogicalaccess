cmake_minimum_required(VERSION 2.6)
project(ISO15693Cards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	ISO15693Cards
	SHARED

	${SRCS}
	${INCLUDE}
  )

target_link_libraries(
	ISO15693Cards
	liblogicalaccess
)