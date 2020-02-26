cmake_minimum_required(VERSION 3.14)
project(RasterTileExtractor)

set(CMAKE_CXX_STANDARD 14)

find_package(GDAL REQUIRED)

add_library(RasterTileExtractor SHARED
        RasterTileExtractor.cpp
        RasterTileExtractor.h)

add_executable(test
        test.cpp)

target_link_libraries(RasterTileExtractor gdal)
target_link_libraries(test RasterTileExtractor)