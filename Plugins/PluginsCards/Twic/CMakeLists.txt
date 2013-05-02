cmake_minimum_required(VERSION 2.6)
project(TwicCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	TwicCards
	SHARED

	${SRCS}
	${INCLUDE}
  )
  
target_link_libraries(
	TwicCards
	ISO7816Cards
	liblogicalaccess
)