cmake_minimum_required(VERSION 2.8)

set(LIBS lmr event_core event_pthreads pthread)

add_executable(netcomm_test netcomm_test.cpp)
target_link_libraries(netcomm_test ${LIBS})

add_executable(register_test register_test.cpp)