cmake_minimum_required(VERSION 3.20)
project(discovery)

find_package(openDAQ REQUIRED)

add_executable(discovery main.cpp)
target_link_libraries(discovery PRIVATE daq::opendaq)
