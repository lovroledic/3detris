#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "shader.hpp"
#include "3detris_camera.hpp"
#include "model.hpp"

#include <iostream>
#include <cmath>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char *path);

const unsigned int SCREEN_WIDTH  = 800;
const unsigned int SCREEN_HEIGHT = 600;
unsigned int currScrWidth  = SCREEN_WIDTH;
unsigned int currScrHeight = SCREEN_HEIGHT;

float deltaTime = 0.0f;
float lastFrameTime = 0.0f;

Camera camera(glm::vec3(0.0f, 17.0f, 0.0f));
float lastX = SCREEN_WIDTH  / 2;
float lastY = SCREEN_HEIGHT / 2;
bool firstMouse = true;

// Game area dimensions
//const unsigned int X = 8, Y = 12, Z = 8;
const int X = 4, Y = 6, Z = 4;
const float xOffset = X / 2. - (X + 1) % 2 / 2.0f,
            yOffset = -0.5f,
            zOffset = Z / 2 - (Z + 1) % 2 / 2.0f;
float blockOffsetX = (X + 1) % 2 / 2.0f;
float blockOffsetZ = (Z + 1) % 2 / 2.0f;
bool moveX = false;
bool moveZ = false;

int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw: window creation
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Ime prozora wowww", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // GLAD: Load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Build and compile shader program
    Shader shader("shaders/my_shader.vert", "shaders/my_shader.frag");

    float vertices[] = {
        // positions          // normals           // texture coords
        // BACK
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f, // 4
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, // 0
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f, // 2
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f, // 4
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f, // 2
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f, // 6
        // FRONT
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f, // 1
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f, // 5
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f, // 7
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f, // 1
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f, // 7
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f, // 3
        // LEFT
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f, // 0
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f, // 1
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f, // 3
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f, // 0
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f, // 3
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f, // 2
        // RIGHT
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, // 5
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f, // 4
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, // 6
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, // 5
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, // 6
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, // 7
        // BOTTOM
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f, // 0
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f, // 4
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f, // 5
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f, // 0
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f, // 5
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f, // 1
        // TOP
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f, // 3
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f, // 7
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f, // 6
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f, // 3
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f, // 6
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f  // 2
    };

    glm::vec3 cubeBlockPositions[] = {
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f,  0.5f),
        glm::vec3(-0.5f,  0.5f, -0.5f),
        glm::vec3(-0.5f,  0.5f,  0.5f),
        glm::vec3( 0.5f, -0.5f, -0.5f),
        glm::vec3( 0.5f, -0.5f,  0.5f),
        glm::vec3( 0.5f,  0.5f, -0.5f),
        glm::vec3( 0.5f,  0.5f,  0.5f),
    };
    glm::vec3 flatBlockPositions[] = {
        glm::vec3(-1.5f, -0.5f, -1.5f),
        glm::vec3(-1.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f, -1.5f),
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f,  0.5f),
        glm::vec3(-0.5f, -0.5f,  1.5f),
        glm::vec3(-1.5f, -0.5f,  0.5f),
        glm::vec3(-1.5f, -0.5f,  1.5f),

        glm::vec3( 1.5f, -0.5f, -1.5f),
        glm::vec3( 1.5f, -0.5f, -0.5f),
        glm::vec3( 0.5f, -0.5f, -1.5f),
        glm::vec3( 0.5f, -0.5f, -0.5f),
        glm::vec3( 0.5f, -0.5f,  0.5f),
        glm::vec3( 0.5f, -0.5f,  1.5f),
        glm::vec3( 1.5f, -0.5f,  0.5f),
        glm::vec3( 1.5f, -0.5f,  1.5f),
    };
    glm::vec3 LBlockPositions[] = {
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f,  0.5f),
        glm::vec3(-0.5f,  0.5f, -0.5f),
        glm::vec3(-0.5f,  0.5f,  0.5f),

        glm::vec3( 0.5f, -0.5f, -0.5f),
        glm::vec3( 0.5f, -0.5f,  0.5f),

        glm::vec3( 1.5f, -0.5f, -0.5f),
        glm::vec3( 1.5f, -0.5f,  0.5f),
    };

    float borderVertices[] = {
        -X / 2.0f,  0.0f,  -Z / 2.0f,
        -X / 2.0f,  0.0f,   Z / 2.0f,

        -X / 2.0f,  0.0f,   Z / 2.0f,
         X / 2.0f,  0.0f,   Z / 2.0f,

         X / 2.0f,  0.0f,   Z / 2.0f,
         X / 2.0f,  0.0f,  -Z / 2.0f,

         X / 2.0f,  0.0f,  -Z / 2.0f,
        -X / 2.0f,  0.0f,  -Z / 2.0f,

        -X / 2.0f,     Y,  -Z / 2.0f,
        -X / 2.0f,     Y,   Z / 2.0f,

        -X / 2.0f,     Y,   Z / 2.0f,
         X / 2.0f,     Y,   Z / 2.0f,

         X / 2.0f,     Y,   Z / 2.0f,
         X / 2.0f,     Y,  -Z / 2.0f,

         X / 2.0f,     Y,  -Z / 2.0f,
        -X / 2.0f,     Y,  -Z / 2.0f,

        -X / 2.0f,  0.0f,  -Z / 2.0f,
        -X / 2.0f,     Y,  -Z / 2.0f,

        -X / 2.0f,  0.0f,   Z / 2.0f,
        -X / 2.0f,     Y,   Z / 2.0f,

         X / 2.0f,  0.0f,   Z / 2.0f,
         X / 2.0f,     Y,   Z / 2.0f,

         X / 2.0f,  0.0f,  -Z / 2.0f,
         X / 2.0f,     Y,  -Z / 2.0f,
    };



    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int lineVBO, lineVAO;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);

    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(borderVertices), borderVertices, GL_STATIC_DRAW);

    glBindVertexArray(lineVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    GLuint redTexture = loadTexture("resources/textures/colors/red.png");
    GLuint greenTexture = loadTexture("resources/textures/colors/green.png");
    GLuint blueTexture = loadTexture("resources/textures/colors/blue.png");
    GLuint whiteTexture = loadTexture("resources/textures/colors/white.png");

    bool positions[X][Y][Z] = { false };
    int rowOccupancyStack[Y] = { 0 }; // arr[x] represents the number of occupied spaces in a row; max X*Z at which point that row is to be freed

    /*const float gravTickLength = 1.0f;
    unsigned int gravTickCount = 0;
    float lastGravTick = 0.0f;
    float secsFromLastTick = 0.0f;*/


    shader.use();
    shader.setFloat("material.shininess", 100.0f);
    shader.setVec3("dirLight.direction", 0.5f, 1.0f, 0.3f);
    shader.setVec3("dirLight.ambient", 0.3f, 0.3f, 0.3f);
    shader.setVec3("dirLight.diffuse", 0.6f, 0.6f, 0.6f);
    shader.setVec3("dirLight.specular", 0.0f, 0.0f, 0.0f);

    Model coloredCube("resources/objects/colored-cube/colored-cube.obj");
    Model whiteBlock("resources/objects/white-block/white-block.obj");
    int tickOffset = 0;
    bool gameActive = true;
    glm::vec3 color = glm::vec3(0.8, 0.8, 0.8);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        float currFrameTime = (float) glfwGetTime();
        deltaTime     = currFrameTime - lastFrameTime;
        lastFrameTime = currFrameTime;

        //std::cout << "delta: " << std::fixed << deltaTime << " s\t" << "FPS: " << std::fixed << ((float) 1 / deltaTime) << "   \r";

        // INPUT
        processInput(window);

        // RENDERING
        glClearColor(color.x, color.y, color.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw
        shader.use();
        shader.setVec3("viewPos", camera.Position);
        
        //glm::vec3 dirLightDir(sin(currFrameTime) / 2, 1.0f, cos(currFrameTime) / 2);
        //shader.setVec3("dirLight.direction", dirLightDir);

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)currScrWidth / currScrHeight, 0.1f, 100.0f);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
    

        glm::mat4 model(1.0f);

        glBindVertexArray(lineVAO);
        shader.setMat4("model", model);
        glDrawArrays(GL_LINES, 0, 24);

        glBindTexture(GL_TEXTURE_2D, blueTexture);
        model = glm::translate(model, glm::vec3(5.0f, 0.0f, 5.0f));
        shader.setMat4("model", model);
        whiteBlock.draw();

        /*{
        // CUBEMAP
            glm::mat4 model(1.0f);
            model = glm::scale(model, glm::vec3(50.0f));
            shader.setMat4("model", model);

            glBindTexture(GL_TEXTURE_2D, whiteTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }*/

        float GAME_SPEED = 1;
        if (gameActive)
        {
            int tick = (int) (currFrameTime * GAME_SPEED) - tickOffset;
    
            model = glm::mat4(1.0f);
            glm::vec3 pos = glm::vec3(blockOffsetX, Y - tick - 0.5, blockOffsetZ);
            model = glm::translate(model, pos);
            shader.setMat4("model", model);

            // Real coordinates converted to indices for positions array
            int xIndex = pos.x + xOffset;
            int yIndex = pos.y + yOffset;
            int zIndex = pos.z + zOffset;

            // Falling/controlled block collided with static block => lock it in place (one block above the block it collided with)
            if (yIndex < 0 || positions[xIndex][yIndex][zIndex])
            {
                positions[xIndex][yIndex + 1][zIndex] = true;

                // Last controlled block got locked in place at the top => GAME OVER
                if (positions[xIndex][Y - 1][zIndex])
                {
                    gameActive = false;
                    color = glm::vec3(0.8f, 0.0f, 0.0f);
                }
                else
                {
                    rowOccupancyStack[yIndex + 1]++;

                    std::cout << "Count per row:" << std::endl;
                    for (int i = 0; i < Y; i++)
                        std::cout << "\t#" << i + 1 << "\t" << rowOccupancyStack[i] << std::endl;

                    // Row in which the last block just got locked in place is filled up => empty it and move everything above it down
                    if (rowOccupancyStack[yIndex + 1] == X * Z)
                    {
                        for (int j = yIndex + 1; j < Y - 1; j++)
                        {
                            rowOccupancyStack[j] = rowOccupancyStack[j + 1]; // this won't update the top row, but it should still always be 0 because otherwise it's game over
                            
                            for (int i = 0; i < X; i++)
                                for (int k = 0; k < Z; k++)
                                    positions[i][j][k] = positions[i][j + 1][k];
                        }

                        std::cout << "!!! Cleared row " << yIndex + 2 << " !!!" << std::endl; 
                    }
                }
                
                tickOffset += tick;
            }
            else
                coloredCube.draw();
        }

        // Draw occupied blocks
        // Must be drawn after the falling block, otherwise a visual stutter occurs
        for (unsigned int i = 0; i < X; i++)
        {
            for (unsigned int j = 0; j < Y; j++)
            {
                for (unsigned int k = 0; k < Z; k++)
                {
                    if (positions[i][j][k])
                    {
                        glm::mat4 model(1.0f);
                        model = glm::translate(model, glm::vec3(((int)i - xOffset), (j - yOffset), ((int)k - zOffset))); // kako bi (0,0,0) bilo točno u sredini XxZ grida
                        shader.setMat4("model", model);
                        whiteBlock.draw();
                    }
                }
            }
        }


        // Check and call events and swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // izbrisat buffere??
        
    shader.del();
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &VBO);


    glfwTerminate();
    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    currScrWidth = width;
    currScrHeight = height;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.processKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.processKeyboard(UP, deltaTime);


    // Handle block control
    // moveX/Z variables make is so the user has to click individually for each movement (you can't hold to move)
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && !moveX)
    {
        if (blockOffsetX + 1 <= xOffset)
            blockOffsetX += 1;
        moveX = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && !moveX)
    {
        if (blockOffsetX - 1 >= -xOffset)
            blockOffsetX -= 1;
        moveX = true;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE)
    {
        moveX = false;
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !moveZ)      //     +----- +x (right)
    {                                                                       //     |
        if (blockOffsetZ + 1 <= zOffset)                               //     |
            blockOffsetZ += 1;                                              //    +z (down)
        moveZ = true;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !moveZ)
    {
        if (blockOffsetZ - 1 >= -zOffset)
            blockOffsetZ -= 1;
        moveZ = true;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE)
    {
        moveZ = false;
    }

    
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // mouse y-coordinates go from top to bottom

    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.processMouseScroll((float) yoffset);
}

unsigned int loadTexture(const char *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, numComponents;
    unsigned char *data = stbi_load(path, &width, &height, &numComponents, 0);
    if (data)
    {
        GLenum format;
        if (numComponents == 1)
            format = GL_RED;
        else if (numComponents == 3)
            format = GL_RGB;
        else if (numComponents == 4)
            format = GL_RGBA;
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}