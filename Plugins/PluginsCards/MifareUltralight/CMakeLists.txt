cmake_minimum_required(VERSION 2.6)
project(MifareUltralightCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	MifareUltralightCards
	SHARED

	${SRCS}
	${INCLUDE}
  )

SET_TARGET_PROPERTIES(MifareUltralightCards PROPERTIES VERSION 2.0.3)

target_link_libraries(
	MifareUltralightCards
	liblogicalaccess
)