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

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

target_link_libraries(
	iCode1Cards
	ISO15693Cards
	${LINK_OPT}
)