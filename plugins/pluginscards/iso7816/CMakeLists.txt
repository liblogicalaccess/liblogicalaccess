cmake_minimum_required(VERSION 3.1)
project(iso7816cards)

if (WIN32)
	SET(VERSION_PRODUCTNAME_VALUE "iso7816cards")
	add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif()

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	iso7816cards
	SHARED
	${SRCS}
	${INCLUDE}
	${resourcerc}
  )

SET_TARGET_PROPERTIES(iso7816cards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	iso7816cards
	${LIBLOGICALACCESS_LIBRARY}
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/iso7816 )
install ( TARGETS iso7816cards RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX} )