cmake_minimum_required(VERSION 3.10)
project(wsstests)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# set(CMAKE_CXX_CLANG_TIDY 
#  clang-tidy
#  -header-filter=.
#  -checks=*
#  -use-color;)

# possibly need to add gcc flags? 
#Taken from https://google.github.io/googletest/quickstart-cmake.html
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(${PROJECT_NAME} tests.cpp contentLengthTests.cpp fixtures.h)
target_link_libraries(${PROJECT_NAME} PUBLIC WSS gtest gtest_main) 
install (TARGETS ${PROJECT_NAME} DESTINATION bin)

# run
# cmake .
# make