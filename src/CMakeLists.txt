cmake_minimum_required (VERSION 2.8)
project (CORESET)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_C_COMPILER mpicc)
SET(CMAKE_CXX_COMPILER mpicxx)

SET(CMAKE_CXX_FLAGS "-std=c++11 -m64")

add_executable (coreset coreset.cpp worker.cpp config.cpp data.cpp)
target_link_libraries(coreset mpi)

add_executable (gen fake_data_gen.cpp config.cpp)

#add_executable (test_eigen test_eigen.cpp)
