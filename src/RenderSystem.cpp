#include "RenderSystem.h"
#include "Debug.h"

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
    textureArrayShader = std::make_unique<Shader>("/Users/robpaslaski/Documents/meincraft/src/ArrayVertex.glsl",
                                                  "/Users/robpaslaski/Documents/meincraft/src/ArrayFragment.glsl");
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
    // might want zFar = function of number of loaded chunks
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 400.0f);
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
    // viewport dimensions in some devices (we're looking blockAt you M1 macs...)
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