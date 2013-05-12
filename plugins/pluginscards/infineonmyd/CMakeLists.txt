cmake_minimum_required(VERSION 2.6)
project(InfineonMYDCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	InfineonMYDCards
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
	InfineonMYDCards
	ISO15693Cards
	${LINK_OPT}
)