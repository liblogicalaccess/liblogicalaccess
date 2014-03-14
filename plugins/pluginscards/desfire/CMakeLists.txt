cmake_minimum_required(VERSION 2.6)
project(desfirecards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	desfirecards
        SHARED	
	${SRCS}
  )

SET_TARGET_PROPERTIES(desfirecards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	desfirecards
	logicalaccess-cryptolib
)
if (MSVC)
  SET_TARGET_PROPERTIES(desfirecards PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:msvcrt.lib")
endif()

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/desfire )
install ( TARGETS desfirecards LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
