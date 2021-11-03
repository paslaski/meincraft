cmake_minimum_required(VERSION 3.17)
project(meincraft)

set(CMAKE_CXX_STANDARD 20)

# Add header file
set(GLEW_H /opt/homebrew/Cellar/glew/2.2.0_1/include/) # GL
set(GLFW_H /opt/homebrew/Cellar/glfw/3.3.4/include/) # GLFW
set(GLAD_H /Users/robpaslaski/Downloads/glad/include/)
set(EXT_H /Users/robpaslaski/Documents/meincraft/ext/)
set(ENTT_H /Users/robpaslaski/Documents/meincraft/entt/)

include_directories(${GLEW_H} ${GLFW_H} ${GLAD_H} ${EXT_H} ${ENTT_H})

# Add target link
set(GLEW_LINK /opt/homebrew/Cellar/glew/2.2.0_1/lib/libGLEW.2.2.dylib)
set(GLFW_LINK /opt/homebrew/Cellar/glfw/3.3.4/lib/libglfw.3.dylib)
link_libraries(${OPENGL} ${GLEW_LINK} ${GLFW_LINK})

## Add header file
#set(GLEW_H /Users/robpaslaski/Downloads/glew-2.1.0/include/) # GL
#set(GLFW_H /Users/robpaslaski/Downloads/glfw-3.3.4.bin.MACOS/include/) # GLFW
#set(GLAD_H /Users/robpaslaski/Downloads/glad/include)
#set(EXT_H /Users/robpaslaski/Documents/Projects/meincraft/ext/)
#include_directories(${GLEW_H} ${GLFW_H} ${GLAD_H} ${EXT_H})
#
## Add target link
#set(GLEW_LINK /usr/local/Cellar/glew/2.2.0_1/lib/libGLEW.2.2.dylib)
#set(GLFW_LINK /Users/robpaslaski/Downloads/glfw-3.3.4.bin.MACOS/lib-universal/libglfw.3.dylib) # 3 diff ones
#link_libraries(${OPENGL} ${GLEW_LINK} ${GLFW_LINK})

# Execute compilation command
set(SOURCE_FILES src/main.cpp)
# added glad to be compiled before main
add_executable(meincraft src/main.cpp /Users/robpaslaski/Downloads/glad/src/glad.c src/main.cpp ext/stb_image.cpp src/entity.cpp src/entity.h src/components.h src/world.cpp src/world.h src/rendersystem.cpp src/rendersystem.h src/inputsystem.cpp src/inputsystem.h src/camera.h src/chunk.cpp src/chunk.h src/block.h src/shader.cpp src/shader.h src/texture.cpp src/texture.h src/debug.h src/block.cpp src/mesh.cpp src/mesh.h src/ChunkMeshingSystem.cpp src/ChunkMeshingSystem.h src/ChunkLoaderSystem.cpp src/ChunkLoaderSystem.h)

if (APPLE)
    target_link_libraries(meincraft "-framework OpenGL")
    target_link_libraries(meincraft "-framework GLUT")
endif()

