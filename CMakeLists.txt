set(CMAKE_MINIMUM_REQUIRED_VERSION 3.18)
set(CMAKE_CXX_STANDARD 14)
project(PROTEI_TEST)

set(HEADER_FILES myLib.h)

add_executable(server.cpp ${HEADER_FILES})
add_executable(client client.cpp ${HEADER_FILES})