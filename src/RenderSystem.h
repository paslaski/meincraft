#pragma once

#include "Camera.h"
#include <GLFW/glfw3.h>
#include <entt.hpp>
#include "Texture.h"
#include "Shader.h"
#include "Components.h"

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

