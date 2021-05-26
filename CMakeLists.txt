cmake_minimum_required(VERSION 3.17)
project(meincraft)

set(CMAKE_CXX_STANDARD 14)

# Add header file
set(GLEW_H /usr/local/Cellar/glew/2.2.0_1/include/) # GL
set(GLFW_H /usr/local/Cellar/glfw/3.3.4/include/) # GLFW
set(GLAD_H /usr/local/include/glad/include/)
set(EXT_H /Users/robpaslaski/Documents/Projects/meincraft/ext/)
include_directories(${GLEW_H} ${GLFW_H} ${GLAD_H} ${EXT_H})

# Add target link
set(GLEW_LINK /usr/local/Cellar/glew/2.2.0_1/lib/libGLEW.2.2.dylib)
set(GLFW_LINK /usr/local/Cellar/glfw/3.3.4/lib/libglfw.3.dylib)
link_libraries(${OPENGL} ${GLEW_LINK} ${GLFW_LINK})

# Execute compilation command
set(SOURCE_FILES src/main.cpp)
# added glad to be compiled before main
add_executable(meincraft src/main.cpp /usr/local/include/glad/src/glad.c src/main.cpp ext/stb_image.cpp)

if (APPLE)
    target_link_libraries(meincraft "-framework OpenGL")
    target_link_libraries(meincraft "-framework GLUT")
endif()

