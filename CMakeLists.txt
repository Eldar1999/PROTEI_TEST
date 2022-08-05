set(CMAKE_MINIMUM_REQUIRED_VERSION 3.18)
set(CMAKE_CXX_STANDARD 20)
project(PROTEI_TEST)

set(HEADER_FILES myLib.h)

add_executable(server server.cpp ${HEADER_FILES})
add_executable(clientUDP client.cpp ${HEADER_FILES})
add_executable(clientTCP client.cpp ${HEADER_FILES})