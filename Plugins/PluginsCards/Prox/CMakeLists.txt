cmake_minimum_required(VERSION 2.6)
project(ProxCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	ProxCards
	SHARED

	${SRCS}
	${INCLUDE}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

SET_TARGET_PROPERTIES(ProxCards PROPERTIES VERSION 2.0.3)

target_link_libraries(
	ProxCards
	${LINK_OPT}
)