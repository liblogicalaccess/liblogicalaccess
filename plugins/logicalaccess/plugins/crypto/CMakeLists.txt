
set(CMAKE_POSITION_INDEPENDENT_CODE ON) 

if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wl,--no-undefined")
else()
  if (MSVC)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  endif ()
endif()

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE HDRS *.h)
file(GLOB_RECURSE HDPPRS *.hpp)

set(INCLUDE ${HDRS} ${HDPPRS} )

add_library(
	logicalaccess-cryptolib
	SHARED
	${SRCS}
	${INCLUDE}
  )

target_link_libraries(logicalaccess-cryptolib PUBLIC ssl crypto llacommon)

 
install ( FILES ${INCLUDE} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/logicalaccess/plugins/crypto )
install ( TARGETS logicalaccess-cryptolib ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX} )
if (MSVC)
	install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif()
