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

add_executable(${PROJECT_NAME} serve_connection.cpp client.cpp request.txt)
target_link_libraries(${PROJECT_NAME} PUBLIC WSS) 
install (TARGETS ${PROJECT_NAME} DESTINATION bin)
