cmake_minimum_required(VERSION 2.6)
project(DESFireCards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.h)

add_library(
	DESFireCards
	SHARED

	${SRCS}
	${INCLUDE}
  )

SET_TARGET_PROPERTIES(DESFireCards PROPERTIES VERSION 2.0.3)

set(OPT_LIBS)
if (WIN32)
  set(OPT_LIBS zlib libeay32 liblogicalaccess)
else()
  set(OPT_LIBS crypto z logicalaccess)
endif()

target_link_libraries(
	DESFireCards
	logicalaccess-cryptolib
	${OPT_LIBS}
)
if (WIN32)
  SET_TARGET_PROPERTIES(DESFireCards PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:msvcrt.lib")
endif()