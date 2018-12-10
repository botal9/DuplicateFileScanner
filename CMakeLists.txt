cmake_minimum_required(VERSION 2.8)

project(dirdemo)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets REQUIRED)
find_package(Boost 1.63.0 REQUIRED)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

add_executable(dirdemo
        src/main.cpp
        src/mainwindow.h
        src/mainwindow.cpp
        src/library/DirectoryScanner.cpp
        src/library/DirectoryScanner.h
        src/library/FileComparator.cpp
        src/library/FileComparator.h src/library/retry.cpp src/library/retry.h src/library/Worker.cpp src/library/Worker.h)

qt5_use_modules(dirdemo Widgets)
