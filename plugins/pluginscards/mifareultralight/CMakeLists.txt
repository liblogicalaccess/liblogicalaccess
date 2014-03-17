cmake_minimum_required(VERSION 2.6)
project(mifareultralightcards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	mifareultralightcards
	SHARED

	${SRCS}
	${INCLUDE}
  )

SET_TARGET_PROPERTIES(mifareultralightcards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	mifareultralightcards
	logicalaccess
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/mifareultralight )
install ( TARGETS mifareultralightcards RUNTIME DESTINATION bin ARCHIVE DESTINATION lib${LIB_SUFFIX}/logicalaccess LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
