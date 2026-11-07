cmake_minimum_required(VERSION 3.22)
project(player VERSION 0.0)

set(NAME "${PROJECT_NAME}" CACHE STRING "Set application name")

add_subdirectory(src)
add_subdirectory(plugin)
