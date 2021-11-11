// NOTE: might replace callbacks with more comprehensive InputSystem events
// probably only necessary if things like inventory implemented

#include "InputSystem.h"

InputSystem::InputSystem() : window(NULL), camera(NULL)
{}

InputSystem::InputSystem(GLFWwindow* w, Camera* c) : window(w), camera(c)
{}

InputSystem::~InputSystem()
{}

void InputSystem::update(entt::registry& registry, double deltaTime)
{
    // poll input events
    glfwPollEvents();
    // act based on last time step & current input
    processInput(deltaTime);
}

void InputSystem::assign_window(GLFWwindow *w)
{
    window = w;
}

void InputSystem::assign_window_callbacks()
{
    // pass member fxn callbacks to window
    // credit: https://stackoverflow.com/questions/7676971/pointing-to-a-function-that-is-a-class-member-glfw-setkeycallback
    glfwSetWindowUserPointer(window, this);

    auto pass_frame_buffer_callback = [](GLFWwindow* w, int wid, int hei)
    {
        static_cast<InputSystem*>(glfwGetWindowUserPointer(w))->framebuffer_size_callback(w, wid, hei);
    };

    auto pass_mouse_callback = [](GLFWwindow* w, double x, double y)
    {
        static_cast<InputSystem*>(glfwGetWindowUserPointer(w))->get_camera()->ProcessMouseMovement(x, y);
    };

    auto pass_scroll_callback = [](GLFWwindow* w, double xoff, double yoff)
    {
        static_cast<InputSystem*>(glfwGetWindowUserPointer(w))->get_camera()->ProcessMouseScroll(yoff);
    };

    glfwSetFramebufferSizeCallback(window, pass_frame_buffer_callback);
    glfwSetCursorPosCallback(window, pass_mouse_callback);
    glfwSetScrollCallback(window, pass_scroll_callback);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void InputSystem::processInput(double deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->MoveCamera(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->MoveCamera(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->MoveCamera(LEFT, deltaTime);
//        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->MoveCamera(RIGHT, deltaTime);
//        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void InputSystem::framebuffer_size_callback(GLFWwindow* w, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}