cmake_minimum_required(VERSION 2.6)
project(pcscreaders)

if (WIN32)
	SET(VERSION_PRODUCTNAME_VALUE "pcscreaders")
	add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif()

set(INCLUDE_FOLDER 	${CMAKE_CURRENT_SOURCE_DIR}/../../pluginsreaderproviders/iso7816
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/desfire
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/mifare
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/twic
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/tagit
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/iso15693
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/mifareultralight
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/mifareplus
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/iso7816
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/samav2
					${CMAKE_CURRENT_SOURCE_DIR}/../../pluginscards/prox
					/usr/include/PCSC)
if (UNIX)
  set(INCLUDE_FOLDER ${INCLUDE_FOLDER} "/usr/local/include/PCSC/")
endif()

INCLUDE_DIRECTORIES(${INCLUDE_FOLDER})


file(GLOB SRCS *.cpp)
file(GLOB include *.hpp)

file(GLOB include_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.hpp)
SOURCE_GROUP("Header Files\\readercardadapters" FILES ${include_readercardadapters})

file(GLOB source_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.cpp)
SOURCE_GROUP("Source Files\\readercardadapters" FILES ${source_readercardadapters})

file(GLOB include_commands ${CMAKE_CURRENT_SOURCE_DIR}/commands/*.hpp)
SOURCE_GROUP("Header Files\\commands" FILES ${include_commands})

file(GLOB source_commands ${CMAKE_CURRENT_SOURCE_DIR}/commands/*.cpp)
SOURCE_GROUP("Source Files\\commands" FILES ${source_commands})

file(GLOB include_readers ${CMAKE_CURRENT_SOURCE_DIR}/readers/*.hpp)
SOURCE_GROUP("Header Files\\readers" FILES ${include_readers})

file(GLOB source_readers ${CMAKE_CURRENT_SOURCE_DIR}/readers/*.cpp)
SOURCE_GROUP("Source Files\\readers" FILES ${source_readers})


set(INCLUDE_LIB ${include} ${include_core} ${include_readercardadapters} ${include_readers} ${include_commands})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters} ${source_readers} ${source_commands})

					
set (LINK_OPT)
if (MSVC)
  set(LINK_OPT winscard )
else (UNIX)
	set(LINK_OPT pcsclite )
endif()

add_library(
	pcscreaders
	SHARED
	
	${SOURCE_LIB}
	${INCLUDE_LIB}
	${resourcerc}
	)

SET_TARGET_PROPERTIES(pcscreaders PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})
  
target_link_libraries(
	pcscreaders
	desfirecards
	mifarecards
	mifarepluscards
	mifareultralightcards
	iso7816readers
	iso7816cards
	iso15693cards
	proxcards
	${LIBLOGICALACCESS_LIBRARY}
	${LINK_OPT}
)

install ( FILES ${include} DESTINATION include/logicalaccess/plugins/readers/pcsc )
install ( FILES ${include_readercardadapters} DESTINATION include/logicalaccess/plugins/readers/pcsc/readercardadapters )
install ( FILES ${include_commands} DESTINATION include/logicalaccess/plugins/readers/pcsc/commands )
install ( FILES ${include_readers} DESTINATION include/logicalaccess/plugins/readers/pcsc/readers )
install ( TARGETS pcscreaders RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX} )
