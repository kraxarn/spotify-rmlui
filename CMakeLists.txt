cmake_minimum_required(VERSION 3.20)
project(spotify-qt-rmlui)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(spotify-qt-rmlui
	main.cpp
	shell/sdl/RenderInterface.hpp
	shell/sdl/SystemInterface.hpp)

# Currently, we rely on SDL
# TODO: Use GLFW/sokol/... instead
include(thirdparty/SDL2.cmake)
target_link_libraries(spotify-qt-rmlui PRIVATE SDL2 SDL2_image)

# RmlUi
include(thirdparty/RmlUi.cmake)
target_link_libraries(spotify-qt-rmlui PRIVATE RmlCore RmlDebugger freetype GL GLEW)
target_compile_definitions(spotify-qt-rmlui PRIVATE RMLUI_STATIC_LIB)