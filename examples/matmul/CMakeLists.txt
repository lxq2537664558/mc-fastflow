include_directories( ${PROJECT_SOURCE_DIR} )
add_executable( matmul matmul.cpp )
add_executable( ff_matmul ff_matmul.cpp )
add_executable( ff_matmul_v2 ff_matmul_v2.cpp )

set( TESTS ff_matmul ff_matmul_v2 )


if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
endif (CMAKE_SYSTEM_NAME MATCHES "Windows")

#if (NOT(CMAKE_SYSTEM_NAME MATCHES "Windows"))
target_link_libraries ( matmul ${CMAKE_THREAD_LIBS_INIT}  )
target_link_libraries ( ff_matmul ${CMAKE_THREAD_LIBS_INIT}  )
target_link_libraries ( ff_matmul_v2 ${CMAKE_THREAD_LIBS_INIT}  )
#
#set_target_properties ( matmul         PROPERTIES COMPILE_FLAGS "-O3" )
#set_target_properties ( ff_matmul      PROPERTIES COMPILE_FLAGS "-O3" )
#set_target_properties ( ff_matmul_v2   PROPERTIES COMPILE_FLAGS "-O3" )
#endif (NOT(CMAKE_SYSTEM_NAME MATCHES "Windows"))

add_test( ff_matmul-3w-with-check    ${CMAKE_CURRENT_BINARY_DIR}/ff_matmul 3 1 )
add_test( ff_matmul-3w-with-check ${CMAKE_CURRENT_BINARY_DIR}/ff_matmul_v2 4 1 )
add_test( ff_matmul-3w-perf    ${CMAKE_CURRENT_BINARY_DIR}/ff_matmul
3)
add_test( ff_matmul_v2-3w-perf    ${CMAKE_CURRENT_BINARY_DIR}/ff_matmul_v2 3)
