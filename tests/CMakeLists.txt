cmake_minimum_required(VERSION 3.14)
project(my_project)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
file(GLOB_RECURSE LIBS ../libs/*.h)

include_directories(../libs)
message(STATUS "FILES: ${LIBS}\n")
add_library(myLib STATIC ${LIBS})
set_target_properties(myLib PROPERTIES LINKER_LANGUAGE CXX)

add_executable(my_project test.cpp ${LIBS})
target_link_libraries(my_project GTest::gtest_main myLib)

include(GoogleTest)
gtest_discover_tests(my_project)

