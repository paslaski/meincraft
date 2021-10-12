#include "world.h"

#include <string>
#include "entity.h"

#include <iostream>

#include <stb_image.h>


World::World()
{
    create_window();
    load_texture_map();
}

World::~World()
{
    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
}

Entity World::create_entity(std::string tag)
{
    Entity entity(registry.create(), this);
    entity.add_component<TagComponent>(tag);
    return entity;
}

void World::render_dirt_system()
{
    // render
    // ------
    glClearColor(0.604f, 0.796f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

    for (auto [entity, position] : registry.view<PositionComponent>().each())
    {
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
        // ourShader.setMat4("projection", projection); // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
        // ourShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position.pos);
//            float angle = 20.0f * i;
//            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
        // ourShader.setMat4("model", model);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void World::create_window()
{
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
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

    if (window == NULL) // error handling
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);

    // pass member fxn callbacks to window
    // credit: https://stackoverflow.com/questions/7676971/pointing-to-a-function-that-is-a-class-member-glfw-setkeycallback
    glfwSetWindowUserPointer(window, this);

    auto pass_frame_buffer_callback = [](GLFWwindow* w, int wid, int hei)
    {
        static_cast<World*>(glfwGetWindowUserPointer(w))->framebuffer_size_callback(w, wid, hei);
    };

    auto pass_mouse_callback = [](GLFWwindow* w, double x, double y)
    {
        static_cast<World*>(glfwGetWindowUserPointer(w))->mouse_callback(w, x, y);
    };

    auto pass_scroll_callback = [](GLFWwindow* w, double xoff, double yoff)
    {
        static_cast<World*>(glfwGetWindowUserPointer(w))->scroll_callback(w, xoff, yoff);
    };

    glfwSetFramebufferSizeCallback(window, pass_frame_buffer_callback);
    glfwSetCursorPosCallback(window, pass_mouse_callback);
    glfwSetScrollCallback(window, pass_scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // We pass GLAD the function to load the address of the OpenGL function pointers which
    // is OS-specific. GLFW gives us glfwGetProcAddress that defines the correct function based
    // on which OS we're compiling for
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    // tell OpenGL the ize of the rendering window so OpenGL knows how we want
    // to display the data and coordinates with respect to the window

    // must pass the frame buffer size bc pixel dimensions don't necessarily match
    // viewport dimensions in some devices (we're looking at you M1 macs...)
    int frameBufferWidth, frameBufferHeight;
    glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
    glViewport(0, 0, frameBufferWidth, frameBufferHeight);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
}

void World::load_texture_map()
{
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

    // create vertex buffer & vertex array objects
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
    unsigned char *data = stbi_load("/Users/robpaslaski/Documents/meincraft/img/texture_atlas.png", &width, &height, &nrChannels, 0);
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

    // took out of render loop: revisit if rendering off texture map
    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureAtlas);

    // activate shader
    glUseProgram(shaderProgram);
}

void World::update()
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    render_dirt_system();
//    processInput(window); // input system?
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void World::processInput(GLFWwindow *window)
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
void World::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void World::mouse_callback(GLFWwindow* window, double xpos, double ypos)
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
void World::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

bool World::isDestroyed()
{
    return glfwWindowShouldClose(window);
}

void checkGLError()
{
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR){
        std::cout << err;
    }
}