cmake_minimum_required(VERSION 3.19)
project (WSS)
include(GNUInstallDirs)

option(ENABLE_TESTING ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# set(CMAKE_CXX_CLANG_TIDY 
#  clang-tidy
#  -header-filter=.
#  -checks=*
#  -use-color;)
add_library(WSS INTERFACE)
target_include_directories(WSS INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>) 

add_subdirectory("tests")
    enable_testing()