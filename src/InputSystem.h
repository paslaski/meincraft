#pragma once

#include "Camera.h"
#include "Block.h"
#include <GLFW/glfw3.h>
#include <entt/entt.hpp>

class Camera;

class InputSystem {
public:
    InputSystem();
    InputSystem(GLFWwindow* w, Camera* c);
    ~InputSystem();

    void update(entt::registry& registry, double deltaTime);

    void assign_window_callbacks();
    Camera* get_camera() { return camera; }

private:
    GLFWwindow* window;
    Camera* camera;

    // OpenGL window callback functions
    void processKeyCallbacks(double deltaTime);
    void framebuffer_size_callback(GLFWwindow* w, int width, int height);

    void processMovement(double deltaTime);
    void processDebug();
//    void processClick();
//    BlockType& selectBlock();

};
