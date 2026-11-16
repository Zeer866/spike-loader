cmake_minimum_required(VERSION 3.22.1)
project(spikeloader)

add_library(spikeloader SHARED loader.cpp)
find_library(log-lib log)
target_link_libraries(spikeloader ${log-lib} dl)
