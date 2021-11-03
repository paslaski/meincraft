#pragma once

#include "camera.h"
#include <GLFW/glfw3.h>
#include <entt.hpp>
#include "texture.h"
#include "shader.h"
#include "components.h"

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

    std::unique_ptr<Texture> textureArray;
    std::unique_ptr<Shader> textureArrayShader;

    void renderChunks(entt::registry& registry);
    void setBlockVAO();
    void bindBuffer(MeshComponent& meshComponent);

    void createWindow();

    static void clear_buffers();

    // settings and constants
    const unsigned int SCR_WIDTH = 800;
    const unsigned int SCR_HEIGHT = 600;

    unsigned int blockVAO;

};

