cmake_minimum_required(VERSION 3.16)
project(keytweak C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(keytweak WIN32
    src/main.c
    src/key_catalog.c
    src/layout_win32.c
    src/remap.c
)

target_include_directories(keytweak PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if(WIN32)
    target_compile_definitions(keytweak PRIVATE KEYTWEAK_PLATFORM_WIN32=1)
    target_link_libraries(keytweak PRIVATE user32 gdi32)
endif()
