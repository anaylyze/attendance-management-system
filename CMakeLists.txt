cmake_minimum_required(VERSION 3.16)
project(AttendanceManagementSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(ams main.cpp ${SOURCES})

if(MSVC)
    target_compile_options(ams PRIVATE /W4 /wd4100)
else()
    target_compile_options(ams PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Copy data/reports dirs to build on first build
add_custom_command(TARGET ams POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ams>/data
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ams>/reports
)
