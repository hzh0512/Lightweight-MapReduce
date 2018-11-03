cmake_minimum_required(VERSION 2.8)
project(lmr_main)

# set(CMAKE_CXX_STANDARD 11)
set(LIBS lmr event_core pthread)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories("/usr/local/include")
link_directories("/usr/local/lib")

add_library(lmr netcomm.cpp netcomm.h common.h)

add_executable(lmr_main main.cpp)
target_link_libraries(lmr_main ${LIBS})