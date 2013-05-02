cmake_minimum_required(VERSION 2.6)
project(InfineonMYDCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	InfineonMYDCards
	SHARED

	${SRCS}
	${INCLUDE}
  )

target_link_libraries(
	InfineonMYDCards
	ISO15693Cards
	liblogicalaccess
)