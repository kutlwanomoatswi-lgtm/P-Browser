cmake_minimum_required(VERSION 3.10)
project(PBrowser CXX)

set(CMAKE_CXX_STANDARD 17)

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/third_party/imgui")

include_directories(
    src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

if(PS5_SDK_PATH)
    message(STATUS "Building with PS5 SDK at: ${PS5_SDK_PATH}")
    include_directories(${PS5_SDK_PATH}/include)
endif()

file(GLOB SOURCES "src/*.cpp")

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

add_executable(eboot.bin ${SOURCES} ${IMGUI_SOURCES})

find_package(SDL2 REQUIRED)
find_package(CURL REQUIRED)

target_link_libraries(eboot.bin PRIVATE 
    SDL2::SDL2
    CURL::libcurl
    pthread
)
