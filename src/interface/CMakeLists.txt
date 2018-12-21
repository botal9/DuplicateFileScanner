cmake_minimum_required(VERSION 2.8)
project(ScannerUI)

include_directories(${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS: "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -D_GLIBCXX_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

include_directories(${CMAKE_SOURCE_DIR}/../library)

link_libraries(ScannerLib)

add_library(ScannerUI STATIC
        mainwindow.h
        mainwindow.cpp
        TreeWidgetItem.h
        TreeWidgetItem.cpp)

qt5_use_modules(ScannerUI Widgets)
