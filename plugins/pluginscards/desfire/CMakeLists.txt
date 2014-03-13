cmake_minimum_required(VERSION 2.6)
project(desfirecards)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})


file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	desfirecards
	MODULE
	${SRCS}
  )

SET_TARGET_PROPERTIES(desfirecards PROPERTIES VERSION 1.70)

target_link_libraries(
	desfirecards
	logicalaccess-cryptolib
	${ZLIB_LIBRARIES}
	${OPENSSL_LIBRARIES}
)
if (MSVC)
  SET_TARGET_PROPERTIES(desfirecards PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:msvcrt.lib")
endif()

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/desfire )
install ( TARGETS liblogicalaccess LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
