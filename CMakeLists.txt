cmake_minimum_required(VERSION 3.25)
project(EvolutionaryAlgorithms)

set(CMAKE_CXX_STANDARD 17)

add_executable(EvolutionaryAlgorithms
        main.cpp
        ant_colony_optimization.cpp
        genetic_algorithm.h genetic_algorithm.cpp)

target_include_directories(EvolutionaryAlgorithms PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )
