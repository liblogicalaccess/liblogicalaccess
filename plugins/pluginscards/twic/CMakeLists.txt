cmake_minimum_required(VERSION 2.6)
project(TwicCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	TwicCards
	SHARED

	${SRCS}
	${INCLUDE}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()
  
target_link_libraries(
	TwicCards
	ISO7816Cards
	${LINK_OPT}
)