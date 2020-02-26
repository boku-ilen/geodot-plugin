cmake_minimum_required(VERSION 3.14)
project(RasterExtractor)

set(CMAKE_CXX_STANDARD 14)

find_package(GDAL REQUIRED)

add_executable(RasterExtractor main.cpp)
target_link_libraries(RasterExtractor gdal)