cmake_minimum_required(VERSION 3.16) # focal
project(player VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-DASIO_STANDALONE -DASIO_NO_DEPRECATED)
add_definitions(-DVERSION="${PROJECT_VERSION}")

add_executable(${PROJECT_NAME}
    src/amcp.hpp
    src/main.cpp
    src/osc.hpp
    src/types.hpp
)
target_link_libraries(${PROJECT_NAME} osc++)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES LICENSE.md README.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
