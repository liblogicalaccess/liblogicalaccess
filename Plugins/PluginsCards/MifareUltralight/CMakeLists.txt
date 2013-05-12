cmake_minimum_required(VERSION 2.6)
project(MifareUltralightCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	MifareUltralightCards
	SHARED

	${SRCS}
	${INCLUDE}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

SET_TARGET_PROPERTIES(MifareUltralightCards PROPERTIES VERSION 2.0.3)

target_link_libraries(
	MifareUltralightCards
	${LINK_OPT}
)