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
  
set(OPT_LIBS)
if(WIN32)
	set(OPT_LIBS ISO7816Cards)
endif()
  
target_link_libraries(
	TwicCards
	${OPT_LIBS}
	liblogicalaccess
)