cmake_minimum_required(VERSION 2.6)
project(twiccards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	twiccards
	SHARED
	${SRCS}
	${INCLUDE}
  )

SET_TARGET_PROPERTIES(twiccards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})
  
target_link_libraries(
	twiccards
	iso7816cards
	${LIBLOGICALACCESS_LIBRARY}
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/twic )
install ( TARGETS twiccards RUNTIME DESTINATION bin/${TARGET_ARCH} ARCHIVE DESTINATION lib/${TARGET_ARCH}/${LIB_SUFFIX}/logicalaccess LIBRARY DESTINATION lib/${TARGET_ARCH}/${LIB_SUFFIX}/logicalaccess )
