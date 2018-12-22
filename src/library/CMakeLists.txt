cmake_minimum_required(VERSION 2.8)
project(ScannerLib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS: "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -D_GLIBCXX_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

include_directories(${CMAKE_SOURCE_DIR})

add_library(ScannerLib STATIC
        DirectoryScanner.cpp
        DirectoryScanner.h
        FileComparator.cpp
        FileComparator.h
        FileComparator.cpp
        FileComparator.h

        retry.cpp
        retry.h
        util.h
        Worker.cpp
        Worker.h)

qt5_use_modules(ScannerLib Widgets)
