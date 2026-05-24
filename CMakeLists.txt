cmake_minimum_required(VERSION 4.2)
project(SkinViewer)

set(CMAKE_CXX_STANDARD 23)

set(SOURCES
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        rlimgui/rlImGui.cpp
        MinecraftSkin.cpp
        UserInterface.cpp
        main.cpp
        FileSystem.cpp
        Renderer.cpp
        Renderer.cpp
        Renderer.h
)

add_executable(SkinViewer ${SOURCES})

target_include_directories(SkinViewer PRIVATE include imgui rlimgui/extras)
target_link_libraries(SkinViewer PRIVATE ssl crypto raylib)
target_compile_definitions(SkinViewer PRIVATE VERSION="0.2")