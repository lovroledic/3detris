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
#include "block.hpp"

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

// Game settings
float GAME_SPEED = 1;
// Game area dimensions
const unsigned int G_X = 8, G_Y = 12, G_Z = 8;
// Game area offset (relative to (0,0,0))
const float G_OFFSET_X = 0;
const float G_OFFSET_Y = 0;
const float G_OFFSET_Z = 0;
// Current player-controlled block area index offset
int bxIndex;
int byIndex;
int bzIndex;
// For preventing continuous movement when holding arrow keys
bool moveX = false;
bool moveZ = false;


Camera camera(glm::vec3(G_X / 2.0f, G_Y + 4, G_Z * 2));
float lastX = SCREEN_WIDTH  / 2;
float lastY = SCREEN_HEIGHT / 2;
bool firstMouse = true;

struct CustomBlock
{
    bool positions[3][3][3];
    int X;
    int Y;
    int Z;
    int rowCount[3];
};

CustomBlock customBlock;

CustomBlock cube = {
    .positions = {
        { // Y = 0
            { 1, 1, 0 },
            { 1, 1, 0 },
            { 0, 0, 0 },
        },
        { // Y = 1
            { 1, 1, 0 },
            { 1, 1, 0 },
            { 0, 0, 0 },
        },
        { // Y = 2
            { 0, 0, 0 },
            { 0, 0, 0 },
            { 0, 0, 0 },
        },
    },
    .X = 2,
    .Y = 2,
    .Z = 2,
    .rowCount = { 4, 4, 0 }
};
CustomBlock stick = {
    .positions = {
        { // Y = 0
            { 1, 0, 0 },
            { 0, 0, 0 },
            { 0, 0, 0 },
        },
        { // Y = 1
            { 1, 0, 0 },
            { 0, 0, 0 },
            { 0, 0, 0 },
        },
        { // Y = 2
            { 1, 0, 0 },
            { 0, 0, 0 },
            { 0, 0, 0 },
        },
    },
    .X = 1,
    .Y = 3,
    .Z = 1,
    .rowCount = { 1, 1, 1 }
};
CustomBlock weird = {
    .positions = {
        { // Y = 0
            { 0, 1, 0 },
            { 1, 1, 0 },
            { 0, 0, 0 },
        },
        { // Y = 1
            { 0, 0, 0 },
            { 0, 1, 0 },
            { 0, 0, 0 },
        },
        { // Y = 2
            { 0, 0, 0 },
            { 0, 0, 0 },
            { 0, 0, 0 },
        },
    },
    .X = 2,
    .Y = 2,
    .Z = 2,
    .rowCount = { 3, 1, 0 }
};
CustomBlock weirdReverse = {
    .positions = {
        { // Y = 0
            { 1, 0, 0 },
            { 0, 0, 0 },
            { 0, 0, 0 },
        },
        { // Y = 1
            { 1, 1, 0 },
            { 1, 0, 0 },
            { 0, 0, 0 },
        },
        { // Y = 2
            { 0, 0, 0 },
            { 0, 0, 0 },
            { 0, 0, 0 },
        },
    },
    .X = 2,
    .Y = 2,
    .Z = 2,
    .rowCount = { 1, 3, 0 }
};

int main()
{
    srand(time(nullptr));

    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw: window creation
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "3Detris", NULL, NULL);
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


    // Game area border
    float borderVertices[] = {
        // bottom square
            G_OFFSET_X - 0.5f,     G_OFFSET_Y - 0.5f,     G_OFFSET_Z - 0.5f,
            G_OFFSET_X - 0.5f,     G_OFFSET_Y - 0.5f, G_Z + G_OFFSET_Z - 0.5f,

            G_OFFSET_X - 0.5f,     G_OFFSET_Y - 0.5f, G_Z + G_OFFSET_Z - 0.5f,
        G_X + G_OFFSET_X - 0.5f,     G_OFFSET_Y - 0.5f, G_Z + G_OFFSET_Z - 0.5f,

        G_X + G_OFFSET_X - 0.5f,     G_OFFSET_Y - 0.5f, G_Z + G_OFFSET_Z - 0.5f,
        G_X + G_OFFSET_X - 0.5f,     G_OFFSET_Y - 0.5f,     G_OFFSET_Z - 0.5f,

        G_X + G_OFFSET_X - 0.5f,     G_OFFSET_Y - 0.5f,     G_OFFSET_Z - 0.5f,
            G_OFFSET_X - 0.5f,     G_OFFSET_Y - 0.5f,     G_OFFSET_Z - 0.5f,

        // top square
            G_OFFSET_X - 0.5f, G_Y + G_OFFSET_Y - 0.5f,     G_OFFSET_Z - 0.5f,
            G_OFFSET_X - 0.5f, G_Y + G_OFFSET_Y - 0.5f, G_Z + G_OFFSET_Z - 0.5f,

            G_OFFSET_X - 0.5f, G_Y + G_OFFSET_Y - 0.5f, G_Z + G_OFFSET_Z - 0.5f,
        G_X + G_OFFSET_X - 0.5f, G_Y + G_OFFSET_Y - 0.5f, G_Z + G_OFFSET_Z - 0.5f,

        G_X + G_OFFSET_X - 0.5f, G_Y + G_OFFSET_Y - 0.5f, G_Z + G_OFFSET_Z - 0.5f,
        G_X + G_OFFSET_X - 0.5f, G_Y + G_OFFSET_Y - 0.5f,     G_OFFSET_Z - 0.5f,

        G_X + G_OFFSET_X - 0.5f, G_Y + G_OFFSET_Y - 0.5f,     G_OFFSET_Z - 0.5f,
            G_OFFSET_X - 0.5f, G_Y + G_OFFSET_Y - 0.5f,     G_OFFSET_Z - 0.5f,

        // vertical lines connecting bottom and top square
            G_OFFSET_X - 0.5f,     G_OFFSET_Y - 0.5f,     G_OFFSET_Z - 0.5f,
            G_OFFSET_X - 0.5f, G_Y + G_OFFSET_Y - 0.5f,     G_OFFSET_Z - 0.5f,

            G_OFFSET_X - 0.5f,     G_OFFSET_Y - 0.5f, G_Z + G_OFFSET_Z - 0.5f,
            G_OFFSET_X - 0.5f, G_Y + G_OFFSET_Y - 0.5f, G_Z + G_OFFSET_Z - 0.5f,

        G_X + G_OFFSET_X - 0.5f,     G_OFFSET_Y - 0.5f, G_Z + G_OFFSET_Z - 0.5f,
        G_X + G_OFFSET_X - 0.5f, G_Y + G_OFFSET_Y - 0.5f, G_Z + G_OFFSET_Z - 0.5f,

        G_X + G_OFFSET_X - 0.5f,     G_OFFSET_Y - 0.5f,     G_OFFSET_Z - 0.5f,
        G_X + G_OFFSET_X - 0.5f, G_Y + G_OFFSET_Y - 0.5f,     G_OFFSET_Z - 0.5f,
    };
    unsigned int lineVBO, lineVAO;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(borderVertices), borderVertices, GL_STATIC_DRAW);
    glBindVertexArray(lineVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    GLuint whiteTexture = loadTexture("resources/textures/colors/white.png");
    GLuint redTexture = loadTexture("resources/textures/colors/red.png");
    GLuint greenTexture = loadTexture("resources/textures/colors/green.png");
    GLuint blueTexture = loadTexture("resources/textures/colors/blue.png");
    GLuint cyanTexture = loadTexture("resources/textures/colors/cyan.png");
    GLuint magentaTexture = loadTexture("resources/textures/colors/magenta.png");
    GLuint yellowTexture = loadTexture("resources/textures/colors/yellow.png");

    GLuint positions[G_X][G_Y][G_Z] = { 0 };
    int rowOccupancyStack[G_Y] = { 0 }; // arr[x] represents the number of occupied spaces in a row; max G_X*Z at which point that row is to be freed


    shader.use();
    shader.setFloat("material.shininess", 100.0f);
    shader.setVec3("dirLight.direction", 0.5f, 1.0f, 0.3f);
    shader.setVec3("dirLight.ambient", 0.3f, 0.3f, 0.3f);
    shader.setVec3("dirLight.diffuse", 0.6f, 0.6f, 0.6f);
    shader.setVec3("dirLight.specular", 0.0f, 0.0f, 0.0f);

    Block block("resources/objects/white-block/white-block.obj", whiteTexture);
    Model coloredBlock("resources/objects/colored-cube/colored-cube.obj");
    Model whiteBlock("resources/objects/white-block/white-block.obj");

    int tickOffset = 0;
    bool gameActive = true;
    glm::vec3 color = glm::vec3(0.6f, 0.6f, 0.6f);

    bool shouldChangeBlock = true;
    GLuint currColor = 0;
    customBlock = cube;

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

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)currScrWidth / currScrHeight, 0.1f, 100.0f);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
    

        glm::mat4 model(1.0f);

        glBindVertexArray(lineVAO);
        shader.setMat4("model", model);
        glDrawArrays(GL_LINES, 0, 24);

        // For navigating where +x/+z is
        model = glm::translate(model, glm::vec3(10.0f, 0.0f, 10.0f));
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

        int tick = 0;
        if (shouldChangeBlock)
        {
            int rb = rand() % 4;
            int rc = rand() % 7;
            
            if (rb == 0) customBlock = cube;
            else if (rb == 1) customBlock = stick;
            else if (rb == 2) customBlock = weird;
            else customBlock = weirdReverse;

            if (rc == 0) currColor = whiteTexture;
            else if (rc == 1) currColor = redTexture;
            else if (rc == 2) currColor = greenTexture;
            else if (rc == 3) currColor = blueTexture;
            else if (rc == 4) currColor = cyanTexture;
            else if (rc == 5) currColor = magentaTexture;
            else currColor = yellowTexture;

            shouldChangeBlock = false;

            bxIndex = (G_X - customBlock.X) / 2;
            bzIndex = (G_Z - customBlock.Z) / 2;
        }

        if (gameActive)
        {
            tick = currFrameTime * GAME_SPEED - tickOffset;
            byIndex = G_Y - customBlock.Y - tick; // Move falling block down by 1 each tick
    
            model = glm::mat4(1.0f);
            glm::vec3 pos = glm::vec3(G_OFFSET_X + bxIndex, G_OFFSET_Y + byIndex, G_OFFSET_Z + bzIndex);


            bool collisionDetected = false;

            // Block reached the ground
            if (byIndex < 0)
                collisionDetected = true;
            else
                // Check if falling block collided with a static block
                for (int j = 0; j < customBlock.Y && !collisionDetected; j++)
                    for (int i = 0; i < customBlock.X && !collisionDetected; i++)
                        for (int k = 0; k < customBlock.Z && !collisionDetected; k++)
                            if (customBlock.positions[j][i][k] && positions[bxIndex + i][byIndex + j][bzIndex + k])
                                collisionDetected = true;
    
            if (collisionDetected)
            {
                // Lock the block in place (one block above the block it collided with)
                for (int j = 0; j < customBlock.Y; j++)
                    for (int i = 0; i < customBlock.X; i++)
                        for (int k = 0; k < customBlock.Z; k++)
                            if (customBlock.positions[j][i][k])
                                //positions[bxIndex + i][byIndex + j + 1][bzIndex + k] = true;
                                positions[bxIndex + i][byIndex + j + 1][bzIndex + k] = currColor;

                // Check if game over
                for (int i = 0; i < customBlock.X; i++)
                {
                    for (int k = 0; k < customBlock.Z; k++)
                    {
                        // Last controlled block got locked in place at the top => GAME OVER
                        if (positions[bxIndex + i][G_Y - 1][bzIndex + k])
                        {
                            gameActive = false;
                            color = glm::vec3(0.8f, 0.0f, 0.0f);
                        }
                    }
                }
                
                if (gameActive)
                {
                    for (int j = 0; j < customBlock.Y; j++)
                        rowOccupancyStack[byIndex + 1 + j] += customBlock.rowCount[j];
                    
                    /* std::cout << "Count per row:" << std::endl;
                    for (int j = 0; j < G_Y; j++)
                        std::cout << "\trow #" << j + 1 << "\t" << rowOccupancyStack[j] << std::endl;  */

                    // Rows in which the last block just got locked in place got filled up => empty them and move everything above them down
                    for (int j = 0; j < customBlock.Y; j++)
                        if (rowOccupancyStack[byIndex + 1 + j] == G_X * G_Z)
                        {
                            for (int jj = byIndex + 1; jj < G_Y - 1; jj++)
                            {
                                rowOccupancyStack[jj] = rowOccupancyStack[jj + 1]; // this won't update the top row, but it should still always be 0 because otherwise it's game over
                                
                                for (int i = 0; i < G_X; i++)
                                    for (int k = 0; k < G_Z; k++)
                                        positions[i][jj][k] = positions[i][jj + 1][k];
                            }

                            std::cout << "!!! Cleared row " << byIndex + 2 << " !!!" << std::endl;
                            j--; // Everything above the cleared row got shifted down -> check the same row again
                        }
                }
                
                tickOffset += tick;
                shouldChangeBlock = true;
            }
            // No collision => draw next frame of falling block
            else
            {
                for (int j = 0; j < customBlock.Y; j++)
                    for (int i = 0; i < customBlock.X; i++)
                        for (int k = 0; k < customBlock.Z; k++)
                            if (customBlock.positions[j][i][k])
                            {
                                model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x + i, pos.y + j, pos.z + k));
                                shader.setMat4("model", model);
                                //coloredBlock.draw();
                                block.textureID = currColor;
                                block.draw();
                            }
            }
        }
        

        // Draw occupied blocks
        // Must be drawn after the falling block, otherwise a visual stutter occurs
        for (int i = 0; i < G_X; i++)
        {
            for (int j = 0; j < G_Y; j++)
            {
                for (int k = 0; k < G_Z; k++)
                {
                    if (positions[i][j][k])
                    {
                        glm::mat4 model(1.0f);
                        model = glm::translate(model, glm::vec3(i + G_OFFSET_X, j + G_OFFSET_Y, k + G_OFFSET_Z)); // kako bi (0,0,0) bilo točno u sredini XxZ grida
                        shader.setMat4("model", model);
                        block.textureID = positions[i][j][k];
                        block.draw();
                        //whiteBlock.draw();
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
    whiteBlock.del();
    coloredBlock.del();
    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &lineVBO);


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
    // moveX/Z variables make it so the user has to click individually for each movement (you can't hold to move)

    // TODO: should be unable to move block into another block
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && !moveX)
    {
        if (bxIndex + customBlock.X < G_X)
            bxIndex += 1;
        moveX = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && !moveX)
    {
        if (bxIndex > 0)
            bxIndex -= 1;
        moveX = true;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE)
    {
        moveX = false;
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !moveZ)      //     +----- +x (right)
    {                                                                       //     |
        if (bzIndex + customBlock.Z < G_Z)                                  //     |
            bzIndex += 1;                                                   //    +z (down)
        moveZ = true;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !moveZ)
    {
        if (bzIndex > 0)
            bzIndex -= 1;
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