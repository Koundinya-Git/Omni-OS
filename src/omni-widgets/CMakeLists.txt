cmake_minimum_required(VERSION 3.16)
project(omni-widgets VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
find_package(LayerShellQt REQUIRED)

add_executable(omni-widgets
    main.cpp
    WidgetEngine.cpp WidgetEngine.h
    Widgets.cpp Widgets.h
)

target_link_libraries(omni-widgets PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    LayerShellQt::Interface
)
