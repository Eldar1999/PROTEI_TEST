cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 20)
project(PROTEI_TEST)

include_directories(libs)
file(GLOB_RECURSE H_FILES libs/*.h)
file(GLOB_RECURSE CPP_FILES libs/*.cpp)

message(STATUS "${H_FILES}")
message(STATUS "${CPP_FILES}")

add_executable(server server.cpp ${H_FILES} ${CPP_FILES})
add_executable(clientUDP client.cpp ${H_FILES} ${CPP_FILES})
add_executable(clientTCP client.cpp ${H_FILES} ${CPP_FILES})