#include "rendersystem.h"
#include "debug.h"

#include <iostream>
#include <glad/glad.h>


RenderSystem::RenderSystem()
{
    // create GLFW window
    createWindow();
    // initialize camera
    camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));

    // instantiate texture array & associated shader for blocks
    textureArray = std::make_unique<Texture>("/Users/robpaslaski/Documents/meincraft/img/texture_atlas.png",
                                             GL_TEXTURE_2D_ARRAY);
    textureArrayShader = std::make_unique<Shader>("/Users/robpaslaski/Documents/meincraft/src/arrayVertex.glsl",
                                                  "/Users/robpaslaski/Documents/meincraft/src/arrayFragment.glsl");
    textureArrayShader->Bind();
    textureArrayShader->SetUniform1i("arrayTexture", 0); // array texture sampler
    textureArrayShader->Unbind();

    // create vertex array object for rendering blocks in chunk
    GLCall(glGenVertexArrays(1, &blockVAO));
}

RenderSystem::~RenderSystem()
{
    // de-allocate all OpenGL resources
    GLCall(glDeleteVertexArrays(1, &blockVAO));

    glfwTerminate();
}

void RenderSystem::update(entt::registry& registry)
{
    clear_buffers();

//    simple_render_chunk(registry);
//    render_dirt_system(registry);
    renderChunks(registry);

    glfwSwapBuffers(window);
}

void RenderSystem::renderChunks(entt::registry& registry)
{
    // bind appropriate texture array, shader, and VAO for blocks
    textureArray->Bind();
    textureArrayShader->Bind();
    GLCall(glBindVertexArray(blockVAO));

    // create transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();

    // pass transformation matrices to the shader
    textureArrayShader->SetUniformMat4f("projection", projection);
    textureArrayShader->SetUniformMat4f("view", view);

    // later when multiple mesh types:
    // master renderer iterates through MeshComp view, feeds chunks to renderChunk, water to renderWater, etc.
    auto meshView = registry.view<MeshComponent>();
    for (const entt::entity& meshEntity : meshView)
    {
        MeshComponent& meshComp = meshView.get<MeshComponent>(meshEntity);
        bindBuffer(meshComp); // bind VBO, send updated data to GPU if necessary
        GLCall(glDrawArrays(GL_TRIANGLES, 0, meshComp.chunkVertices.size())); // draw call
    }

    // unbind, don't want persistent side effect
    textureArray->Unbind();
    textureArrayShader->Unbind();
}

void RenderSystem::bindBuffer(MeshComponent& meshComponent)
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, meshComponent.blockVBO)); // bind VBO
    if (meshComponent.mustUpdateBuffer) // only send new data to GPU if necessary
    {
        GLCall(glBufferData(GL_ARRAY_BUFFER, meshComponent.chunkVertices.size() * sizeof(texArrayVertex),
                     &meshComponent.chunkVertices[0], GL_STATIC_DRAW));
        meshComponent.mustUpdateBuffer = false; // note that buffer doesn't need updating until changed
    }
    setBlockVAO();
}

void RenderSystem::setBlockVAO()
{
    // generate OpenGL VAO object
    GLCall(glGenVertexArrays(1, &blockVAO));
    GLCall(glBindVertexArray(blockVAO));

    // position attribute (3 GLfloats)
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(texArrayVertex), (void*)0));
    GLCall(glEnableVertexAttribArray(0));

    // texture coord attribute (3 GLfloats)
    // do we need to specify last as int instead of float?
    GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(texArrayVertex), (void*)offsetof(texArrayVertex, uTexCoord)));
    GLCall(glEnableVertexAttribArray(1));
}

void RenderSystem::simple_render_chunk(entt::registry& registry)
{
    // wireframe for debugging
//    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    std::vector<texArrayVertex> chunkVertices;
    float uvCoords[] = {
            0.0f, 0.0f, // first triangle on face
            1.0f, 0.0f,
            0.0f, 1.0f,
            0.0f, 1.0f, // second triangle on face
            1.0f, 1.0f,
            1.0f, 0.0f
    };
    // aligns direction with faces 0-5 in loop
    // WEST, DOWN, NORTH, EAST, UP, SOUTH
    Direction dir[] = {
            WEST, DOWN, NORTH, EAST, UP, SOUTH
    };

    textureArrayShader->Bind();
    textureArray->Bind();
    set_chunk_vao();

    // create transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();

    // pass transformation matrices to the shader
    textureArrayShader->SetUniformMat4f("projection", projection);
    textureArrayShader->SetUniformMat4f("view", view);

    // range for to iterate over registry view, returning all chunks w/ components
    for (auto [entity, position, blocks] : registry.view<PositionComponent, BlockComponent>().each() )
    {
//        auto& cur_ent_pos = registry.get<PositionComponent>(entity);
//        std::cout << cur_ent_pos.pos.x << std::endl;
        // position.pos refers to (i, j, k) = (0, 0, 0) in world coordinates
        // i, j, k refer to block position in chunk coordinates
        for (int k = 0; k < CHUNK_SIZE; k++)
            for (int j = 0; j < CHUNK_SIZE; j++)
                for (int i = 0; i < CHUNK_SIZE; i++)
                    for (int face = 0; face < 6; face++) // 6 faces for each cube
                        for (int v = 0; v < 6; v++) { // 6 vertices for each face
                            int dim = face % 3;
                            // follows UV coordinates of texture across 2D face surface
                            // also aligns UV with position in world space
                            float xVertOffset, yVertOffset, zVertOffset;
                            if (dim == 0) {
                                xVertOffset = 0.0f;
                                yVertOffset = uvCoords[2*v + 1];
                                zVertOffset = uvCoords[2*v];
                            } else if (dim == 1) {
                                xVertOffset = uvCoords[2*v];
                                yVertOffset = 0.0f;
                                zVertOffset = uvCoords[2*v + 1];
                            } else { // dim == 2
                                xVertOffset = uvCoords[2*v];
                                yVertOffset = uvCoords[2*v + 1];
                                zVertOffset = 0.0f;
                            }
                            // offsets by 1 in x, y, and z direction to create parallel faces
                            float xFaceOffset = (face == 3) ? 1.0f : 0.0f;
                            float yFaceOffset = (face == 4) ? 1.0f : 0.0f;
                            float zFaceOffset = (face == 5) ? 1.0f : 0.0f;
                            // need to support different textures by side
                            // current dim scheme (face = 0 through 5)
                            // WEST, DOWN, NORTH, EAST, UP, SOUTH
                            chunkVertices.emplace_back(
                                    //                           mesh corner + block corner + 1/0 + UV
                                    static_cast<GLfloat>(position.pos.x + i + xVertOffset + xFaceOffset),
                                    static_cast<GLfloat>(position.pos.y + j + yVertOffset + yFaceOffset),
                                    static_cast<GLfloat>(position.pos.z + k + zVertOffset + zFaceOffset),
                                    uvCoords[v*2], uvCoords[v*2 + 1],
                                    static_cast<GLfloat>(sideLookup(blocks.at(i, j, k), dir[face]))
                                    );
                        }
    }
    set_chunk_vbo(chunkVertices);
    glDrawArrays(GL_TRIANGLES, 0, chunkVertices.size());
}

void RenderSystem::set_chunk_vao()
{
    glGenVertexArrays(1, &cVAO);
    glBindVertexArray(cVAO);
    // position attribute (3 GLfloats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(texArrayVertex), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coord attribute (3 GLfloats)
    // do we need to specify last as int instead of float?
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(texArrayVertex), (void*)offsetof(texArrayVertex, uTexCoord));
    glEnableVertexAttribArray(1);
}

void RenderSystem::set_chunk_vbo(std::vector<texArrayVertex>& vertices)
{
    glGenBuffers(1, &cVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cVBO);
    // depends on hasUpdated?
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(texArrayVertex), &vertices[0], GL_STATIC_DRAW);
    // update mustUpdateBuffer after draw
}

void RenderSystem::createWindow()
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