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

    std::shared_ptr<Texture> textureAtlas;
    std::shared_ptr<Shader> simpleShader;
    std::shared_ptr<Texture> textureArray;
    std::shared_ptr<Shader> textureArrayShader;

    void create_window();
    void load_cube_vbo_vao();
    void load_cube_vbo_vao_texture_array();

    void set_chunk_vao();
    void set_chunk_vbo(std::vector<texArrayVertex>& vertices);

    static void clear_buffers();
    void simple_render_chunk(entt::registry& registry);
    void render_dirt_system(entt::registry& registry);

    // settings and constants
    const unsigned int SCR_WIDTH = 800;
    const unsigned int SCR_HEIGHT = 600;

    unsigned int VBO, VAO;
    unsigned int cVBO, cVAO;

};

