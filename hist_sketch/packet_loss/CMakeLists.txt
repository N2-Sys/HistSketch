cmake_minimum_required(VERSION 3.11)

project(hist_sketch_packet_loss CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/)

add_executable(exp_pl src/experiment.cpp)

target_compile_options(exp_pl PUBLIC -O2)