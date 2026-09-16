cmake_minimum_required(VERSION 3.20)
project(streaming_discovery)

find_package(openDAQ REQUIRED)

add_executable(streaming_discovery main.cpp)
target_link_libraries(streaming_discovery PRIVATE daq::opendaq)
