#pragma once

#include <entt.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "components.h"

class Entity;

class World
{
private:
    entt::registry registry;

    void move_system();
    void render_dirt_system();

    void create_window();
    void load_texture_map();

    // OpenGL window callback functions
    void processInput(GLFWwindow *window);
    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    // a little error handling
    void checkGLError();

    // settings and constants
    const unsigned int SCR_WIDTH = 800;
    const unsigned int SCR_HEIGHT = 600;

    // timing
    float deltaTime = 0.0f;	// time between current frame and last frame
    float lastFrame = 0.0f;

    // camera
    glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

    bool firstMouse = true;
    float yaw   = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
    float pitch =  0.0f;
    float lastX =  800.0f / 2.0;
    float lastY =  600.0 / 2.0;
    float fov   =  45.0f;

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

    GLFWwindow* window;

public:
    World();
    ~World();

    Entity create_entity(std::string tag);

    void update();

    bool isDestroyed();

    friend class Entity;
};