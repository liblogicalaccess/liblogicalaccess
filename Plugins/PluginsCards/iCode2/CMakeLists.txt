cmake_minimum_required(VERSION 2.6)
project(iCode2Cards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	iCode2Cards
	SHARED

	${SRCS}
	${INCLUDE}
  )

set(OPT_LIBS)
if(WIN32)
	set(OPT_LIBS ISO15693Cards)
endif()
  
target_link_libraries(
	iCode2Cards
	${OPT_LIBS}
	liblogicalaccess
)