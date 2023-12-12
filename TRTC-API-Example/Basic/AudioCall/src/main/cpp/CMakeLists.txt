cmake_minimum_required(VERSION 3.18.1)

project("tencentcrypy")

add_library(tencentcrypy SHARED CustomerCryptJNI.cpp)

include_directories(./include)

find_library(log-lib log)

target_link_libraries(tencentcrypy ${log-lib})