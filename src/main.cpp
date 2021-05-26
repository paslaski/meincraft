#include <iostream>
#include <fstream>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <math.h>
#include <stb_image.h>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

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

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

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

int main() {
    glfwInit();
    // configure GLFW from enum list before instantiating window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    // supposedly necessary for OSX
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // creating an OpenGL window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) // error handling
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // We pass GLAD the function to load the address of the OpenGL function pointers which
    // is OS-specific. GLFW gives us glfwGetProcAddress that defines the correct function based
    // on which OS we're compiling for
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell OpenGL the ize of the rendering window so OpenGL knows how we want
    // to display the data and coordinates with respect to the window
    glViewport(0, 0, 800, 600);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // cube vertices: position, texture coord
    int textureIdx = 16*15 + 2; // spans 0-255 with 0 at bottom left, 255 at top right
    int row = textureIdx % 16, col = textureIdx / 16;
    float vertices[] = {
            -0.5f, -0.5f, -0.5f,  (float)row/16.0f, (float)col/16.0f,
            0.5f, -0.5f, -0.5f,   ((float)row+1)/16.0f, (float)col/16.0f,
            0.5f,  0.5f, -0.5f,   ((float)row+1)/16.0f, ((float)col+1)/16.0f,
            0.5f,  0.5f, -0.5f,   ((float)row+1)/16.0f,  ((float)col+1)/16.0f,
            -0.5f,  0.5f, -0.5f,  (float)row/16.0f, ((float)col+1)/16.0f,
            -0.5f, -0.5f, -0.5f,  (float)row/16.0f, (float)col/16.0f,

            -0.5f, -0.5f,  0.5f,  (float)row/16.0f, (float)col/16.0f,
            0.5f, -0.5f,  0.5f,   ((float)row+1)/16.0f, (float)col/16.0f,
            0.5f,  0.5f,  0.5f,   ((float)row+1)/16.0f, ((float)col+1)/16.0f,
            0.5f,  0.5f,  0.5f,   ((float)row+1)/16.0f, ((float)col+1)/16.0f,
            -0.5f,  0.5f,  0.5f,  (float)row/16.0f, ((float)col+1)/16.0f,
            -0.5f, -0.5f,  0.5f,  (float)row/16.0f, (float)col/16.0f,

            -0.5f,  0.5f,  0.5f,  ((float)row+1)/16.0f, (float)col/16.0f,
            -0.5f,  0.5f, -0.5f,  ((float)row+1)/16.0f, ((float)col+1)/16.0f,
            -0.5f, -0.5f, -0.5f,  (float)row/16.0f, ((float)col+1)/16.0f,
            -0.5f, -0.5f, -0.5f,  (float)row/16.0f, ((float)col+1)/16.0f,
            -0.5f, -0.5f,  0.5f,  (float)row/16.0f, (float)col/16.0f,
            -0.5f,  0.5f,  0.5f,  ((float)row+1)/16.0f, (float)col/16.0f,

            0.5f,  0.5f,  0.5f,  ((float)row+1)/16.0f, (float)col/16.0f,
            0.5f,  0.5f, -0.5f,  ((float)row+1)/16.0f, ((float)col+1)/16.0f,
            0.5f, -0.5f, -0.5f,  (float)row/16.0f, ((float)col+1)/16.0f,
            0.5f, -0.5f, -0.5f,  (float)row/16.0f, ((float)col+1)/16.0f,
            0.5f, -0.5f,  0.5f,  (float)row/16.0f, (float)col/16.0f,
            0.5f,  0.5f,  0.5f,  ((float)row+1)/16.0f, (float)col/16.0f,

            -0.5f, -0.5f, -0.5f,  (float)row/16.0f, ((float)col+1)/16.0f,
            0.5f, -0.5f, -0.5f,   ((float)row+1)/16.0f, ((float)col+1)/16.0f,
            0.5f, -0.5f,  0.5f,   ((float)row+1)/16.0f, (float)col/16.0f,
            0.5f, -0.5f,  0.5f,   ((float)row+1)/16.0f, (float)col/16.0f,
            -0.5f, -0.5f,  0.5f,  (float)row/16.0f, (float)col/16.0f,
            -0.5f, -0.5f, -0.5f,  (float)row/16.0f, ((float)col+1)/16.0f,

            -0.5f,  0.5f, -0.5f,  (float)row/16.0f, ((float)col+1)/16.0f,
            0.5f,  0.5f, -0.5f,   ((float)row+1)/16.0f, ((float)col+1)/16.0f,
            0.5f,  0.5f,  0.5f,   ((float)row+1)/16.0f, (float)col/16.0f,
            0.5f,  0.5f,  0.5f,   ((float)row+1)/16.0f, (float)col/16.0f,
            -0.5f,  0.5f,  0.5f,  (float)row/16.0f, (float)col/16.0f,
            -0.5f,  0.5f, -0.5f,  (float)row/16.0f, ((float)col+1)/16.0f,
    };
    // world space positions of our cubes
    glm::vec3 cubePositions[] = {
            glm::vec3( 0.0f,  0.0f,  0.0f),
            glm::vec3( 2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3( 2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f,  3.0f, -7.5f),
            glm::vec3( 1.3f, -2.0f, -2.5f),
            glm::vec3( 1.5f,  2.0f, -2.5f),
            glm::vec3( 1.5f,  0.2f, -1.5f),
            glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    // create vertex buffer & vertex array objects
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // load and create a texture
    // -------------------------
    unsigned int textureAtlas;

    glGenTextures(1, &textureAtlas);
    glBindTexture(GL_TEXTURE_2D, textureAtlas);

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *data = stbi_load("/Users/robpaslaski/Documents/Projects/meincraft/img/texture_atlas.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);



    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------

    // create shader object
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // checking for successful shader compilation, printing errors
    int  success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // create fragment shader object + compile shader
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // error handling
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // shader program: object that is the final linked version of multiple shaders combined
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // error handling,
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "textureAtlas"), 0);

    // flip textures
    stbi_set_flip_vertically_on_load(true);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureAtlas);

        // activate shader
        glUseProgram(shaderProgram);

        // create transformations
//        glm::mat4 view          = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
//        glm::mat4 projection    = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
//        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
//        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        // pass transformation matrices to the shader

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
//        ourShader.setMat4("projection", projection); // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
//        ourShader.setMat4("view", view);

        // render boxes
        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 10; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
//            ourShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}