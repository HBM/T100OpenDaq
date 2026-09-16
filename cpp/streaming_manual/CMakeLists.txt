cmake_minimum_required(VERSION 3.20)
project(streaming_manual)

find_package(openDAQ REQUIRED)

add_executable(streaming_manual main.cpp)
target_link_libraries(streaming_manual PRIVATE daq::opendaq)
