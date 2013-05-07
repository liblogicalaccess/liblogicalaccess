cmake_minimum_required(VERSION 2.6)
project(ISO15693Cards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	ISO15693Cards
	SHARED

	${SRCS}
	${INCLUDE}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

SET_TARGET_PROPERTIES(ISO15693Cards PROPERTIES VERSION 2.0.3)

target_link_libraries(
	ISO15693Cards
	${LINK_OPT}
)