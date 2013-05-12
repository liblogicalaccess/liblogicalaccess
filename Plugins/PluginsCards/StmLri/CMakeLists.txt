cmake_minimum_required(VERSION 2.6)
project(StmLri512Cards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	StmLri512Cards
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
	StmLri512Cards
	${LINK_OPT}
)