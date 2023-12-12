cmake_minimum_required(VERSION 3.18.1)

project("tencentcrypy")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}")

add_library(tencentcrypy SHARED src/main/cpp/CustomerCryptJNI.cpp)

include_directories(src/main/cpp/include)

find_library(log-lib log)

target_link_libraries(
        tencentcrypy
        crypto
        ssl
        ${log-lib})