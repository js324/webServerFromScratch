cmake_minimum_required(VERSION 3.10)
project(WSS)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_CLANG_TIDY 
  clang-tidy
  -header-filter=.
  -checks=*
  -use-color;)
add_executable(${PROJECT_NAME} httpserver/ThreadedHTTPServer.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC httpserver PUBLIC httpserver/html/stock_resps)
#INTERFACE adds httpserver folder to interface_include directories, when this (a library) is linked to a target, cmake will search through these dirs to see build properties
#PRIVATE adds to include 
#should later do add_library intead of add_executable

install (TARGETS WSS DESTINATION bin)