cmake_minimum_required(VERSION 3.14)
project(RasterTileExtractor)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INSTALL_RPATH "\$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH on)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_library(GDAL gdal)

add_library(RasterTileExtractor SHARED
        RasterTileExtractor.cpp
        RasterTileExtractor.h)

add_executable(test
        test.cpp)

target_link_libraries(RasterTileExtractor gdal)
target_link_libraries(test RasterTileExtractor)