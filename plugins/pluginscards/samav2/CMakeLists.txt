cmake_minimum_required(VERSION 2.6)
project(samav2cards)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})


file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

file(GLOB include_commands ${CMAKE_CURRENT_SOURCE_DIR}/commands/*.hpp)
SOURCE_GROUP("Header Files\\commands" FILES ${include_commands})

file(GLOB source_commands ${CMAKE_CURRENT_SOURCE_DIR}/commands/*.cpp)
SOURCE_GROUP("Source Files\\commands" FILES ${source_commands})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../desfire)

if (WIN32)
  LINK_DIRECTORIES(${LINK_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../desfire/bin/${TARGET_ARCH}/${VC_FOLDER_NAME})
else()
  LINK_DIRECTORIES(${LINK_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../desfire)
endif()

add_library(
	samav2cards
	SHARED

	${SRCS}
	${source_commands}
	${INCLUDE}
	${include_commands}
  )

SET_TARGET_PROPERTIES(samav2cards PROPERTIES VERSION 1.70)

set(OPT_LIBS)
if (WIN32)
  set(OPT_LIBS liblogicalaccess)
else()
  set(OPT_LIBS logicalaccess)
endif()

target_link_libraries(
	samav2cards
	desfirecards
	logicalaccess-cryptolib
  ${ZLIB_LIBRARIES}
  ${OPENSSL_LIBRARIES}
	${OPT_LIBS}
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/samav2 )
install ( TARGETS samav2cards LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
