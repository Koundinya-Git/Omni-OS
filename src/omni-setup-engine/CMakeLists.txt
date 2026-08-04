cmake_minimum_required(VERSION 3.16)
project(omni-setup-engine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Multimedia)

add_executable(omni-setup-engine
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    resources.qrc
)

target_link_libraries(omni-setup-engine PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Multimedia
)
