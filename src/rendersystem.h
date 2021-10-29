#pragma once

#include "camera.h"
#include <GLFW/glfw3.h>
#include <entt.hpp>
#include "texture.h"
#include "shader.h"

class Camera;

class RenderSystem {
public:

    RenderSystem();
    ~RenderSystem();

    void update(entt::registry& registry);
    GLFWwindow* get_window() { return window; };
    Camera* get_camera() { return &camera; };

private:

    GLFWwindow* window;
    Camera camera;

    std::unique_ptr<Texture> textureAtlas;
    std::unique_ptr<Shader> simpleShader;

    void create_window();
    void load_texture_map();
    void load_texture_map_old();

    static void clear_buffers();
    void simple_render_chunk(entt::registry& registry);
    void render_dirt_system(entt::registry& registry);
    void render_dirt_system_old(entt::registry& registry);

    // settings and constants
    const unsigned int SCR_WIDTH = 800;
    const unsigned int SCR_HEIGHT = 600;

    const char *vertexShaderSource = "#version 330 core\n"
                                     "layout (location = 0) in vec3 aPos;\n"
                                     "layout (location = 1) in vec2 aTexCoord;\n"

                                     "out vec2 TexCoord;\n"

                                     "uniform mat4 model;\n"
                                     "uniform mat4 view;\n"
                                     "uniform mat4 projection;\n"

                                     "void main()\n"
                                     "{\n"
                                     "gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
                                     "TexCoord = aTexCoord;\n"
                                     "}\0";

    const char *fragmentShaderSource = "#version 330 core\n"
                                       "out vec4 FragColor;\n"

                                       "in vec3 ourColor;\n"
                                       "in vec2 TexCoord;\n"

                                       "uniform sampler2D ourTexture;\n"

                                       "void main()\n"
                                       "{\n"
                                       "FragColor = texture(ourTexture, TexCoord);\n"
                                       "}\0";

    unsigned int VBO, VAO;
    unsigned int shaderProgram;

};

