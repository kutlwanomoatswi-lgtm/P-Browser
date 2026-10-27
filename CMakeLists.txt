cmake_minimum_required(VERSION 3.10)
project(PBrowser CXX)

set(CMAKE_CXX_STANDARD 17)

# Include src directory so #include <imgui.h> and backend headers resolve correctly
include_directories(src)

# Automatically collect browser code AND ImGui source files in src/
file(GLOB SOURCES "src/*.cpp")

add_executable(eboot.bin ${SOURCES})

find_package(SDL2 REQUIRED)
target_link_libraries(eboot.bin PRIVATE SDL2::SDL2)
