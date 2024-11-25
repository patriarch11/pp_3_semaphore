cmake_minimum_required(VERSION 3.10)

project(ParallelProgrammingSemaphore)

set(CMAKE_C_STANDARD 99)

add_executable(main main.c semaphore.c)

find_package(PkgConfig REQUIRED)
pkg_check_modules(UUID REQUIRED uuid)
include_directories(${UUID_INCLUDE_DIRS})
target_link_libraries(main ${UUID_LIBRARIES} pthread m)