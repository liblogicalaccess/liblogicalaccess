cmake_minimum_required(VERSION 2.6)
project(stmlri512cards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	stmlri512cards
	SHARED

	${SRCS}
	${INCLUDE}
  )

SET_TARGET_PROPERTIES(stmlri512cards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	stmlri512cards
	logicalaccess
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/stmlri512 )
install ( TARGETS stmlri512cards LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
