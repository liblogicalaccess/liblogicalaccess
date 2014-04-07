cmake_minimum_required(VERSION 2.6)
project(rplethreaders)

if (NOT TARGET_ARCH)
	set(TARGET_ARCH "Win32")
endif()

set(INCLUDE_FOLDER 	${CMAKE_CURRENT_SOURCE_DIR}/../../pluginsreaderproviders/iso7816
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/desfire
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/iso7816
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/samav2)

INCLUDE_DIRECTORIES(${INCLUDE_FOLDER})

file(GLOB SRCS *.cpp)
file(GLOB include *.hpp)

file(GLOB include_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.hpp)
SOURCE_GROUP("Header Files\\readercardadapters" FILES ${include_readercardadapters})

file(GLOB source_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.cpp)
SOURCE_GROUP("Source Files\\readercardadapters" FILES ${source_readercardadapters})

set(INCLUDE_LIB ${include} ${include_core} ${include_readercardadapters})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters})

add_library(
	rplethreaders
	SHARED
	${SOURCE_LIB}
	${INCLUDE_LIB}
  )

SET_TARGET_PROPERTIES(rplethreaders PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})
  
target_link_libraries(
	rplethreaders
	desfirecards
	iso7816readers
	iso7816cards
	${LIBLOGICALACCESS_LIBRARY}
)

install ( FILES ${include} DESTINATION include/logicalaccess/plugins/readers/rpleth )
install ( FILES ${include_readercardadapters} DESTINATION include/logicalaccess/plugins/readers/rpleth/readercardadapters )
install ( TARGETS rplethreaders RUNTIME DESTINATION bin/${TARGET_ARCH} ARCHIVE DESTINATION lib/${TARGET_ARCH}/${LIB_SUFFIX}/logicalaccess LIBRARY DESTINATION lib/${TARGET_ARCH}/${LIB_SUFFIX}/logicalaccess )
