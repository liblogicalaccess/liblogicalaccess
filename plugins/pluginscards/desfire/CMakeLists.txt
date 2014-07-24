cmake_minimum_required(VERSION 2.6)
project(desfirecards)

if (WIN32)
	SET(VERSION_PRODUCTNAME_VALUE "desfirecards")
	add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif()

set(INCLUDE_FOLDER 	${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/iso7816)

INCLUDE_DIRECTORIES(${INCLUDE_FOLDER})

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	desfirecards
    SHARED	
	${SRCS}
	${INCLUDE}
	${resourcerc}
  )

if (WIN32)
  set(LINK_OPT ${LIBLOGICALACCESS_LIBRARY})
endif()
  
SET_TARGET_PROPERTIES(desfirecards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	desfirecards
	logicalaccess-cryptolib
	iso7816cards
	${LINK_OPT}
)
if (MSVC)
  SET_TARGET_PROPERTIES(desfirecards PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:msvcrt.lib")
endif()

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/desfire )
install ( TARGETS desfirecards RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX} )
