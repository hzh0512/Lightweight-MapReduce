cmake_minimum_required(VERSION 2.8)
project(lmr)

set(LIBS lmr event_core event_pthreads pthread ssh)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories("/usr/local/include" "/usr/include" "~/private/libssh/include")
link_directories("/usr/local/lib" "/lib" "~/private/libssh/lib")

add_subdirectory(src)
add_subdirectory(tests)