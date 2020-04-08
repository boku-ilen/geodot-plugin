cmake_minimum_required(VERSION 3.14)
project(VectorExtractor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INSTALL_RPATH "\$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH on)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_library(GDAL gdal)

add_library(VectorExtractor SHARED
        VectorExtractor.cpp
        VectorExtractor.h
        LineFeature.cpp
        LineFeature.h Feature.cpp Feature.h)

add_executable(test
        test.cpp)

target_link_libraries(VectorExtractor gdal)
target_link_libraries(test VectorExtractor)