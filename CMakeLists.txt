cmake_minimum_required(VERSION 3.17)
project(meincraft)

set(CMAKE_CXX_STANDARD 20)

# Add header file
# Must update GLFW_H to directories containing headers
set(GLFW_H /opt/homebrew/Cellar/glfw/3.3.6/include/) # GLFW
set(GLAD_H ext/glad/include/)
set(EXT_H /Users/robpaslaski/Documents/meincraft/ext/)

include_directories( ${GLFW_H} ${GLAD_H} ${EXT_H})

# Add target link
# Must update GLFW_LINK to dynamic library files
set(GLFW_LINK /opt/homebrew/Cellar/glfw/3.3.6/lib/libglfw.3.dylib)
link_libraries(${OPENGL} ${GLFW_LINK})

# Execute compilation command
set(SOURCE_FILES src/main.cpp)
add_executable(meincraft src/main.cpp ext/glad/src/glad.c src/main.cpp ext/stb_image.cpp src/Components.h src/World.cpp src/World.h src/RenderSystem.cpp src/RenderSystem.h src/InputSystem.cpp src/InputSystem.h src/Camera.h src/Chunk.h src/Block.h src/Shader.cpp src/Shader.h src/Texture.cpp src/Texture.h src/Debug.h src/Block.cpp src/ChunkMeshingSystem.cpp src/ChunkMeshingSystem.h src/ChunkLoaderSystem.cpp src/ChunkLoaderSystem.h src/ChunkGenerator.cpp src/ChunkGenerator.h src/Biome.cpp src/Biome.h src/BlockPool.h src/BlockPool.cpp src/Player.cpp src/Player.h)

if (APPLE)
    target_link_libraries(meincraft "-framework OpenGL")
    target_link_libraries(meincraft "-framework GLUT")
endif()

