cmake_minimum_required(VERSION 2.6)
project(desfirecards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	desfirecards
	SHARED

	${SRCS}
	${INCLUDE}
  )

SET_TARGET_PROPERTIES(desfirecards PROPERTIES VERSION 1.67)

set(OPT_LIBS)
if (WIN32)
  set(OPT_LIBS zlib libeay32 liblogicalaccess)
else()
  set(OPT_LIBS crypto z logicalaccess)
endif()

target_link_libraries(
	desfirecards
	logicalaccess-cryptolib
	${OPT_LIBS}
)
if (WIN32)
  SET_TARGET_PROPERTIES(desfirecards PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:msvcrt.lib")
endif()