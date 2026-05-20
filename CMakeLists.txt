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
)

add_executable(SkinViewer ${SOURCES})

target_include_directories(SkinViewer PRIVATE include imgui rlimgui/extras)
target_link_libraries(SkinViewer PRIVATE ssl crypto raylib)

add_custom_command(
        TARGET SkinViewer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:SkinViewer>/assets"
)