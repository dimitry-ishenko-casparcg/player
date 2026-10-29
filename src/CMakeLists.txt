# src

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(pgm_args REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libuv REQUIRED IMPORTED_TARGET libuv)

add_definitions(-DVERSION="${PROJECT_VERSION}")

add_executable(player
    main.cpp
    player.cpp
    server.cpp
    uv/process.cpp
)
target_link_libraries(player PRIVATE pgm::args PkgConfig::libuv)

include(GNUInstallDirs)
install(TARGETS ${name} DESTINATION ${CMAKE_INSTALL_BINDIR})
