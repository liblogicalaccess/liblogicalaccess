cmake_minimum_required(VERSION 2.6)
project(em4135cards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	em4135cards
	SHARED
	${SRCS}
	${INCLUDE}
  )

SET_TARGET_PROPERTIES(em4135cards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	em4135cards
	logicalaccess
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/em4135 )
install ( TARGETS em4135cards RUNTIME DESTINATION bin ARCHIVE DESTINATION lib${LIB_SUFFIX}/logicalaccess LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
