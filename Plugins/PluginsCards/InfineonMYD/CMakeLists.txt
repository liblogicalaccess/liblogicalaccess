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

set(OPT_LIBS)
if(WIN32)
	set(OPT_LIBS ISO15693Cards)
endif()
  
target_link_libraries(
	InfineonMYDCards
	${OPT_LIBS}
	liblogicalaccess
)