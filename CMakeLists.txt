cmake_minimum_required(VERSION 2.8)
project(lmr)

set(LIBS lmr event_core event_pthreads pthread ssh)

if (${CMAKE_SYSTEM_NAME} STREQUAL "CYGWIN")
    set(CMAKE_CXX_STANDARD 14)
else()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(CMAKE_BUILD_TYPE Debug)

include_directories("/usr/local/include" "/usr/include" "~/private/libssh/include")
link_directories("/usr/local/lib" "/lib" "~/private/libssh/lib")

add_subdirectory(src)
add_subdirectory(tests)