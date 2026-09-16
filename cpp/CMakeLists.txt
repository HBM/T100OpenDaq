cmake_minimum_required(VERSION 3.20)
project(T100OpenDaqExamples)

# Requires the openDAQ(TM) SDK to be installed / discoverable via find_package(openDAQ)
# See: https://docs.opendaq.com for SDK installation instructions.

add_subdirectory(streaming_manual)
add_subdirectory(streaming_discovery)
