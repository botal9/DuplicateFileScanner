cmake_minimum_required(VERSION 2.8)

project(DuplicateFileScannerMain)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS: "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -D_GLIBCXX_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

find_package(Qt5Widgets REQUIRED)
find_package(Boost 1.68.0 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/interface)
include_directories(${CMAKE_SOURCE_DIR}/library)

add_subdirectory(library)
add_subdirectory(interface)

add_executable(ScannerMain
        main.cpp)


target_link_libraries(ScannerMain
        ScannerLib
        ScannerUI)

qt5_use_modules(ScannerMain Widgets)
