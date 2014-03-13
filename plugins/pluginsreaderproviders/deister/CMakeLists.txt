cmake_minimum_required(VERSION 2.6)
project(deisterreaders)

find_package(Boost 1.49 REQUIRED COMPONENTS filesystem system)
include_directories(${Boost_INCLUDE_DIRS})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})


file(GLOB SRCS *.cpp)
file(GLOB include *.hpp)

file(GLOB include_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.hpp)
SOURCE_GROUP("Header Files\\readercardadapters" FILES ${include_readercardadapters})

file(GLOB source_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.cpp)
SOURCE_GROUP("Source Files\\readercardadapters" FILES ${source_readercardadapters})

set(INCLUDE_LIB ${include} ${include_core} ${include_readercardadapters})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters})

add_library(
	deisterreaders
	SHARED
	
	${SOURCE_LIB}
	${INCLUDE_LIB}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess)
endif()

SET_TARGET_PROPERTIES(deisterreaders PROPERTIES VERSION 1.70)
  
target_link_libraries(
	deisterreaders
	logicalaccess-cryptolib
	${OPENSSL_LIBRARIES}
	${Boost_LIBRARIES}
	${LINK_OPT}
)

install ( FILES ${include} DESTINATION include/logicalaccess/plugins/readers/deister )
install ( FILES ${include_readercardadapters} DESTINATION include/logicalaccess/plugins/readers/deister/readercardadapters )
install ( TARGETS deisterreaders LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
