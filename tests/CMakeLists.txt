cmake_minimum_required(VERSION 2.8)

set(LIBS lmr event_core event_pthreads pthread ssh)

add_executable(netcomm_test netcomm_test.cpp)
target_link_libraries(netcomm_test ${LIBS})

add_executable(register_test register_test.cpp)
target_link_libraries(register_test ${LIBS})

add_executable(mapinput_test mapinput_test.cpp)
target_link_libraries(mapinput_test ${LIBS})

add_executable(mapper_test mapper_test.cpp)
target_link_libraries(mapper_test ${LIBS})

add_executable(reducer_test reducer_test.cpp)
target_link_libraries(reducer_test ${LIBS})

add_executable(mapreduce_hello_test mapreduce_hello_test.cpp)
target_link_libraries(mapreduce_hello_test ${LIBS})

add_executable(mapreduce_test mapreduce_test.cpp)
target_link_libraries(mapreduce_test ${LIBS})

add_executable(naivebayes_test ml/naivebayes_test.cpp)
target_link_libraries(naivebayes_test ${LIBS})

add_executable(kmeans_test ml/kmeans_test.cpp)
target_link_libraries(kmeans_test ${LIBS})

add_executable(linearregression_test ml/linearregression_test.cpp)
target_link_libraries(linearregression_test ${LIBS})

add_executable(logisticregression_test ml/logisticregression_test.cpp)
target_link_libraries(logisticregression_test ${LIBS})
