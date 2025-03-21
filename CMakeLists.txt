cmake_minimum_required(VERSION 3.16)

project(yt_dlp_gui)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Enable MOC and UIC for Qt6
set(CMAKE_AUTOMOC ON)

add_executable(yt_dlp_gui main.cpp settings_window.h settings_window.cpp)
target_link_libraries(yt_dlp_gui Qt6::Widgets)