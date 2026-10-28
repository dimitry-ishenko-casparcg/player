cmake_minimum_required(VERSION 3.22)
project(player VERSION 0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-DVERSION="${PROJECT_VERSION}")

find_package(pgm_args REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libuv REQUIRED IMPORTED_TARGET libuv)

add_executable(player main.cpp)
target_link_libraries(player PRIVATE pgm::args PkgConfig::libuv)

include(GNUInstallDirs)
install(TARGETS ${name} DESTINATION ${CMAKE_INSTALL_BINDIR})
