cmake_minimum_required(VERSION 3.20)
project(torque_speed_reader)

find_package(openDAQ REQUIRED)

add_executable(torque_speed_reader main.cpp)
target_link_libraries(torque_speed_reader PRIVATE daq::opendaq)
