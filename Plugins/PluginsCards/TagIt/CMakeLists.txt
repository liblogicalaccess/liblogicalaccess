cmake_minimum_required(VERSION 2.6)
project(TagItCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	TagItCards
	SHARED

	${SRCS}
	${INCLUDE}
  )

target_link_libraries(
	TagItCards
	ISO15693Cards
	liblogicalaccess
)