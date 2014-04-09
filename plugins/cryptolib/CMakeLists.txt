
set(CMAKE_POSITION_INDEPENDENT_CODE ON) 

if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
else()
  if (MSVC)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  endif ()
endif()

file(GLOB_RECURSE SRCS src/*.cpp)
file(GLOB_RECURSE HDRS include/*.h)
file(GLOB_RECURSE HDPPRS include/*.hpp)

set(INCLUDE ${HDRS} ${HDPPRS} )

add_library(
	logicalaccess-cryptolib
	STATIC
	${SRCS}
	${INCLUDE}
  )
  
 
install ( FILES ${INCLUDE} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/crypto )

