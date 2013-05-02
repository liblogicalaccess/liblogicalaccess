cmake_minimum_required(VERSION 2.6)
project(iCode1Cards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	iCode1Cards
	SHARED

	${SRCS}
	${INCLUDE}
  )

target_link_libraries(
	iCode1Cards
	${OPT_LIBS}
	ISO15693Cards
	liblogicalaccess
)