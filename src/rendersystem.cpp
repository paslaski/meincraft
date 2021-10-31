#include "rendersystem.h"
#include "components.h"

#include <iostream>
#include <glad/glad.h>
#include <stb_image.h>


RenderSystem::RenderSystem()
{
    // initialize camera
    camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));

    // create GLFW window and prepare block textures
    create_window();
    load_cube_vbo_vao();

//    textureAtlas = std::make_shared<Texture>("/Users/robpaslaski/Documents/meincraft/img/texture_atlas.png", GL_TEXTURE_2D);
//    simpleShader = std::make_shared<Shader>("/Users/robpaslaski/Documents/meincraft/src/simpleVertex.glsl",
//                        "/Users/robpaslaski/Documents/meincraft/src/simpleFragment.glsl");
//    simpleShader->Bind();
//    simpleShader->SetUniform1i("ourTexture", 0);
//    textureAtlas->Bind();

    textureArray = std::make_shared<Texture>("/Users/robpaslaski/Documents/meincraft/img/texture_atlas.png", GL_TEXTURE_2D_ARRAY);
    textureArrayShader = std::make_shared<Shader>("/Users/robpaslaski/Documents/meincraft/src/arrayVertex.glsl",
                                            "/Users/robpaslaski/Documents/meincraft/src/arrayFragment.glsl");
    textureArrayShader->Bind();
    textureArrayShader->SetUniform1i("arrayTexture", 0);
    textureArray->Bind();

    set_chunk_vao();
}

RenderSystem::~RenderSystem()
{
    // de-allocate all resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // clear previously allocated GLFW resources.
    glfwTerminate();
}

void RenderSystem::update(entt::registry& registry)
{
    clear_buffers();

    simple_render_chunk(registry);
//    render_dirt_system(registry);

    glfwSwapBuffers(window);
}

void RenderSystem::simple_render_chunk(entt::registry& registry)
{
    texArrayVertex chunkVertices[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
    float uvCoords[] = {
            0.0f, 0.0f, // first triangle on face
            1.0f, 0.0f,
            0.0f, 1.0f,
            0.0f, 1.0f, // second triangle on face
            1.0f, 1.0f,
            1.0f, 0.0f
    };
    Direction dir[] = {
            SOUTH, WEST, DOWN, NORTH, EAST, UP
    };

    // create transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();

    // pass transformation matrices to the shader
    simpleShader->SetUniformMat4f("projection", projection);
    simpleShader->SetUniformMat4f("view", view);

    // range for to iterate over registry view, returning all chunks w/ components
    for (auto [entity, position, blocks] : registry.view<PositionComponent, BlockComponent>().each() )
    {
        // position.pos refers to (i, j, k) = (0, 0, 0) in world coordinates
        // i, j, k refer to block position in chunk coordinates
        for (int k = 0; k < CHUNK_SIZE; k++)
            for (int j = 0; j < CHUNK_SIZE; j++)
                for (int i = 0; i < CHUNK_SIZE; i++)
                    for (int face = 0; face < 6; face++) // 6 faces for each cube, must change vals
                        for (int v = 0; v < 6; v++) { // 6 times for vertices, must change vals
                            int dim = face % 3;
                            float xVertOffset = (dim==0) ? 0.0f : uvCoords[dim]; // align offset with uv
                            float yVertOffset = (dim==1) ? 0.0f : uvCoords[dim];
                            float zVertOffset = (dim==2) ? 0.0f : uvCoords[dim];
                            float xFaceOffset = (dim==0 && face > 2) ? 1.0f : 0.0f; // repeat for other 3 faces of cube
                            float yFaceOffset = (dim==1 && face > 2) ? 1.0f : 0.0f;
                            float zFaceOffset = (dim==2 && face > 2) ? 1.0f : 0.0f;
                            // need to support different textures by side
                            // current dim scheme (face = 0 through 5)
                            // SOUTH, WEST, DOWN, NORTH, EAST, UP
                            chunkVertices[i + (j * CHUNK_SIZE) + (k * CHUNK_SIZE * CHUNK_SIZE)] =
                                    {
                                    .worldPos = {static_cast<GLfloat>(position.pos.x + i + xVertOffset + xFaceOffset),
                                                  static_cast<GLfloat>(position.pos.y + j + yVertOffset + yFaceOffset),
                                                  static_cast<GLfloat>(position.pos.z + k + zVertOffset + zFaceOffset)},
                                    .textureCoords = {uvCoords[v*2], uvCoords[v*2 + 1],
                                                      static_cast<GLfloat>( sideLookup(blocks.at(i, j, k), dir[face]) )}
                                    };
                        }
    }
    set_chunk_vbo(chunkVertices);
    glDrawArrays(GL_TRIANGLES, 0, 36*CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
}

void RenderSystem::set_chunk_vao()
{

}

void RenderSystem::set_chunk_vbo(texArrayVertex verts[])
{

}

void RenderSystem::render_dirt_system(entt::registry& registry)
{

    for (auto [entity, position] : registry.view<PositionComponent>().each())
    {
        // create transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // pass transformation matrices to the shader
        // note: currently we set the projection matrix each frame, but since the projection
        // matrix rarely changes it's often best practice to set it outside the main loop only once.
        simpleShader->SetUniformMat4f("projection", projection);
        simpleShader->SetUniformMat4f("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position.pos);

        simpleShader->SetUniformMat4f("model",  model);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

}

void RenderSystem::create_window()
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
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "MeinCraft", NULL, NULL);

    if (window == NULL) // error handling
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);

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

void RenderSystem::clear_buffers() {
    glClearColor(0.604f, 0.796f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!
}

void RenderSystem::load_cube_vbo_vao()
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

}

void RenderSystem::load_cube_vbo_vao_texture_array()
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

}