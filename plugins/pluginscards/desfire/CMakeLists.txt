cmake_minimum_required(VERSION 2.6)
project(desfirecards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	desfirecards
    SHARED	
	${SRCS}
	${INCLUDE}
  )

if (WIN32)
  set(LINK_OPT logicalaccess)
endif()
  
SET_TARGET_PROPERTIES(desfirecards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	desfirecards
	logicalaccess-cryptolib
	${LINK_OPT}
)
if (MSVC)
  SET_TARGET_PROPERTIES(desfirecards PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:msvcrt.lib")
endif()

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/desfire )
install ( TARGETS desfirecards RUNTIME DESTINATION bin ARCHIVE DESTINATION lib${LIB_SUFFIX}/logicalaccess LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
