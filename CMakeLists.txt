cmake_minimum_required(VERSION 3.16)

project(YTMusicDownloader)

set(CMAKE_CXX_STANDARD 17)

# Enable MOC, UIC, RCC for Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Required for .app bundle
set(CMAKE_MACOSX_BUNDLE ON)

# Find Qt6 Widgets
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Define the app bundle
add_executable(YTMusicDownloader MACOSX_BUNDLE
    main.cpp
    settings_window.cpp
    settings_window.h
)

# Link Qt libraries
target_link_libraries(YTMusicDownloader PRIVATE Qt6::Widgets)
set_target_properties(YTMusicDownloader PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist"
)

# Optional: Add a custom Info.plist
# set_target_properties(YTMusicDownloader PROPERTIES
#     MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist"
# )