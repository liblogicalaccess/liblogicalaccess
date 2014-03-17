cmake_minimum_required(VERSION 2.6)
project(icode2cards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	icode2cards
	SHARED
	${SRCS}
  )

SET_TARGET_PROPERTIES(icode2cards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	icode2cards
	iso15693cards
	logicalaccess
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/icode2 )
install ( TARGETS icode2cards ARCHIVE DESTINATION lib${LIB_SUFFIX}/logicalaccess LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
