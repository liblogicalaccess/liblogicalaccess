cmake_minimum_required(VERSION 2.6)
project(tagitcards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	tagitcards
	SHARED

	${SRCS}
	${INCLUDE}
  )

SET_TARGET_PROPERTIES(tagitcards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	tagitcards
	iso15693cards
	${LIBLOGICALACCESS_LIBRARY}
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/tagit )
install ( TARGETS tagitcards RUNTIME DESTINATION bin/${TARGET_ARCH} ARCHIVE DESTINATION lib/${TARGET_ARCH}/${LIB_SUFFIX}/logicalaccess LIBRARY DESTINATION lib/${TARGET_ARCH}/${LIB_SUFFIX}/logicalaccess )
