cmake_minimum_required(VERSION 3.20)
project(PSBrowser CXX)

set(CMAKE_CXX_STANDARD 17)

# PacBrew sysroot
set(PACBREW_ROOT "${CMAKE_SOURCE_DIR}/.deps/pacbrew/target/user/homebrew")

include_directories(
    src
    ${CMAKE_SOURCE_DIR}/third_party/imgui
    ${CMAKE_SOURCE_DIR}/third_party/imgui/backends
    ${PACBREW_ROOT}/include
)

file(GLOB SOURCES "src/*.cpp")

set(IMGUI_SOURCES
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/backends/imgui_impl_sdl2.cpp
    third_party/imgui/backends/imgui_impl_sdlrenderer2.cpp
)

add_executable(eboot.elf ${SOURCES} ${IMGUI_SOURCES})

target_link_libraries(eboot.elf PRIVATE 
    ${PACBREW_ROOT}/lib/libSDL2.a
    ${PACBREW_ROOT}/lib/libSDL2_image.a
    ${PACBREW_ROOT}/lib/libcurl.a
    ${PACBREW_ROOT}/lib/libssl.a
    ${PACBREW_ROOT}/lib/libcrypto.a
    -lkernel
    -lSceSystemService
)
