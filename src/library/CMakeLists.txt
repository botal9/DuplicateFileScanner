cmake_minimum_required(VERSION 2.8)
project(ScannerLib)

include_directories(${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS: "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -D_GLIBCXX_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

add_library(ScannerLib STATIC
        Consumer.cpp
        Consumer.h
        DirectoryScanner.cpp
        DirectoryScanner.h
        FileComparator.cpp
        FileComparator.h
        Producer.cpp
        Producer.h
        Scanner.cpp
        Scanner.h

        retry.cpp
        retry.h
        util.h)

qt5_use_modules(ScannerLib Widgets)
