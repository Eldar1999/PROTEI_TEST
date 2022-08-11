cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined")
project(PROTEI_TEST)

include_directories(libs tests)
file(GLOB_RECURSE LIBS libs/*.h)
file(GLOB_RECURSE TESTS tests/*.cpp)

include(tests/CMakeLists.txt)

add_library(myLib STATIC ${LIBS})
set_target_properties(myLib PROPERTIES LINKER_LANGUAGE CXX)

add_executable(server source/server.cpp ${LIBS})
target_link_libraries(server myLib)

add_executable(clientUDP source/client.cpp ${LIBS})
target_link_libraries(clientUDP myLib)

add_executable(clientTCP source/client.cpp ${LIBS})
target_link_libraries(clientTCP myLib)

add_executable(test_exec ${TESTS})
target_link_libraries(test_exec GTest::gtest_main myLib)

gtest_discover_tests(test_exec)


