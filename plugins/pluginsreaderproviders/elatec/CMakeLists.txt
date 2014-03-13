cmake_minimum_required(VERSION 2.6)
project(elatecreaders)

if (UNIX)
  find_package(Boost 1.49 REQUIRED COMPONENTS filesystem system)
  include_directories(${Boost_INCLUDE_DIRS})
endif ()

file(GLOB SRCS *.cpp)
file(GLOB include *.hpp)

file(GLOB include_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.hpp)
SOURCE_GROUP("Header Files\\readercardadapters" FILES ${include_readercardadapters})

file(GLOB source_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.cpp)
SOURCE_GROUP("Source Files\\readercardadapters" FILES ${source_readercardadapters})

set(INCLUDE_LIB ${include} ${include_core} ${include_readercardadapters})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters})

add_library(
	elatecreaders
	SHARED
	
	${SOURCE_LIB}
	${INCLUDE_LIB}
  )
  
if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess ${Boost_LIBRARIES})
endif()

SET_TARGET_PROPERTIES(elatecreaders PROPERTIES VERSION 1.70)
  
target_link_libraries(
	elatecreaders
	${LINK_OPT}
)

install ( FILES ${include} DESTINATION include/logicalaccess/plugins/readers/elatec )
install ( FILES ${include_readercardadapters} DESTINATION include/logicalaccess/plugins/readers/elatec/readercardadapters )
install ( TARGETS elatecreaders LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
