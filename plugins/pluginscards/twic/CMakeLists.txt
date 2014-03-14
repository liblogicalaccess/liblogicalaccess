cmake_minimum_required(VERSION 2.6)
project(twiccards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	twiccards
	SHARED
	${SRCS}
  )

SET_TARGET_PROPERTIES(twiccards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})
  
target_link_libraries(
	twiccards
	iso7816cards
	logicalaccess
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/twic )
install ( TARGETS twiccards LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
