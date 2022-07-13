cmake_minimum_required(VERSION 3.5)
project(spotify-rmlui LANGUAGES CXX VERSION 0.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
add_executable(spotify-rmlui)
add_subdirectory(src)

# GLFW
# TODO: Currently needs to be installed in system
find_package(glfw3 REQUIRED)

# LunaSVG
add_subdirectory(thirdparty/lunasvg)

# RmlUi backend (GLFW/OpenGL3)
target_include_directories(${PROJECT_NAME} PRIVATE
	thirdparty/RmlUi/Backends)

target_sources(${PROJECT_NAME} PRIVATE
	thirdparty/RmlUi/Backends/RmlUi_Backend_GLFW_GL3.cpp
	thirdparty/RmlUi/Backends/RmlUi_Platform_GLFW.cpp
	thirdparty/RmlUi/Backends/RmlUi_Renderer_GL3.cpp)

# RmlUi
include(thirdparty/RmlUi.cmake)
target_link_libraries(spotify-rmlui PRIVATE RmlCore RmlDebugger freetype glfw lunasvg)
target_compile_definitions(spotify-rmlui PRIVATE RMLUI_STATIC_LIB)