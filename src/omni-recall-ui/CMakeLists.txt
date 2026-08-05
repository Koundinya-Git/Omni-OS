cmake_minimum_required(VERSION 3.16)
project(omni_recall_ui VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Sql Network REQUIRED)

add_executable(omni_recall_ui
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(omni_recall_ui PRIVATE 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets 
    Qt6::Sql
    Qt6::Network
)
