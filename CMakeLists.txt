cmake_minimum_required(VERSION 3.20)
project(lab1_KasianchukAnna_k25 CXX)

set(CMAKE_CXX_STANDARD 23)

add_executable(lab1_KasianchukAnna_k25 lab1_v3.cpp
        lab1_v3.cpp)

configure_file(input.txt input.txt COPYONLY)