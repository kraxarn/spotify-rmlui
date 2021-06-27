# Currently only supports SDL2 installed in system

# pkg-config required by SDL2
find_package(PkgConfig REQUIRED)

# SDL2
pkg_check_modules(SDL2 REQUIRED sdl2)
include_directories(${SDL2_INCLUDE_DIRS})
link_directories(${SDL2_LIBRARY_DIRS})

# SDL2 Image
pkg_check_modules(SDL2image REQUIRED SDL2_image)
include_directories(${SDL2image_INCLUDE_DIRS})
link_directories(${SDL2image_LIBRARY_DIRS})

# GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
link_directories(${GLEW_LIBRARY_DIRS})