cmake_minimum_required(VERSION 3.10)
project(PBrowser CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)
file(GLOB SOURCES "src/*.cpp")

add_executable(eboot.bin ${SOURCES})

# Link SDL2 libraries for UI and input management
find_package(SDL2 REQUIRED)
target_link_libraries(eboot.bin PRIVATE SDL2::SDL2)
