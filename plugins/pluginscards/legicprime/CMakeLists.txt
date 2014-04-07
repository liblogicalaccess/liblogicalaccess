cmake_minimum_required(VERSION 2.6)
project(legicprimecards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	legicprimecards
	SHARED
	${SRCS}
	${INCLUDE}
  )

SET_TARGET_PROPERTIES(legicprimecards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	legicprimecards
	${LIBLOGICALACCESS_LIBRARY}
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/legicprime )
install ( TARGETS legicprimecards RUNTIME DESTINATION bin/${TARGET_ARCH} ARCHIVE DESTINATION lib/${TARGET_ARCH}/${LIB_SUFFIX}/logicalaccess LIBRARY DESTINATION lib/${TARGET_ARCH}/${LIB_SUFFIX}/logicalaccess )
