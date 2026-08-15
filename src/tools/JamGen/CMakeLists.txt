cmake_minimum_required(VERSION 3.16)
project(JamGen CXX)

add_executable(JamGen
    JamGen.cpp
    JamGenParser.cpp
)

target_compile_features(JamGen PRIVATE cxx_std_17)
