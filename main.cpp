#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "shader.hpp"
#include "game_logic.hpp"
#include "camera.hpp"
#include "block.hpp"

#include <iostream>
#include <cmath>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H  


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char *path);

void initFreeType();
void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color);


const unsigned int SCREEN_WIDTH  = 800;
const unsigned int SCREEN_HEIGHT = 800;
unsigned int currScrWidth  = SCREEN_WIDTH;
unsigned int currScrHeight = SCREEN_HEIGHT;

float deltaTime = 0.0f;
float lastFrameTime = 0.0f;

Game game;
Camera camera;
float lastX = SCREEN_WIDTH  / 2;
float lastY = SCREEN_HEIGHT / 2;
bool firstMouse = true;

struct Character
{
    unsigned int textureID;
    glm::ivec2   size;
    glm::ivec2   bearing;
    FT_Pos advance;
};
std::map<char, Character> characters;
unsigned int fVAO, fVBO;

int main()
{
    srand(time(nullptr));

    // glfw: initialize and configure
    glfwInit(); // NOTE: generating any buffers before this causes a segmentation fault, generally when initializing objects in global scope
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

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
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
    // for fonts
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // ------------------------------------------------------------------------------------------------
    game.init();
    camera = Camera(game.area);
    block = Block("resources/objects/block/white-block.obj", 0);

    // Build and compile shader program
    Shader shader("shaders/my_shader.vert", "shaders/my_shader.frag");
    Shader lightSourceShader("shaders/my_shader.vert", "shaders/light_source.frag");
    Shader textShader("shaders/text.vert", "shaders/text.frag");
    
    initFreeType();
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(currScrWidth), 0.0f, static_cast<float>(currScrHeight));
    textShader.use();
    textShader.setMat4("projection", projection);

    glm::vec3 areaCenter = glm::vec3( AREA_WIDTH / 2.0f - 0.5f, AREA_HEIGHT / 2.0f - 0.5f, AREA_WIDTH / 2.0f - 0.5f);

    shader.use();
    shader.setInt("material.diffuse", 0); // 0 == GL_TEXTURE0
    shader.setInt("material.specular", 1); // 1 == GL_TEXTURE1
    shader.setFloat("material.shininess", 100.0f);
    shader.setVec3("dirLight.direction", 0.2f, 1.0f, 0.2f);
    shader.setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
    shader.setVec3("dirLight.diffuse", 0.8f, 0.8f, 0.8f);
    shader.setVec3("dirLight.specular", 0.3f, 0.3f, 0.3f);
    /* shader.setVec3("dirLight.ambient", 0.0f, 0.0f, 0.0f);
    shader.setVec3("dirLight.diffuse", 0.0f, 0.0f, 0.0f);
    shader.setVec3("dirLight.specular", 0.0f, 0.0f, 0.0f); */

    shader.setVec3("pointLights[0].ambient", 0.0f, 0.0f, 0.0f);
    shader.setVec3("pointLights[0].diffuse", 0.6f, 0.6f, 0.6f);
    shader.setVec3("pointLights[0].specular", 0.5f, 0.5f, 0.5f);
    shader.setFloat("pointLights[0].constant", 1.0f);
    shader.setFloat("pointLights[0].linear", 0.045);
    shader.setFloat("pointLights[0].quadratic", 0.0075);

    shader.setVec3("pointLights[1].ambient", 0.0f, 0.0f, 0.0f);
    shader.setVec3("pointLights[1].diffuse", 1.0f, 0.0f, 0.0f);
    shader.setVec3("pointLights[1].specular", 0.5f, 0.0f, 0.0f);
    shader.setFloat("pointLights[1].constant", 1.0f);
    shader.setFloat("pointLights[1].linear", 0.045);
    shader.setFloat("pointLights[1].quadratic", 0.0075);

    shader.setVec3("pointLights[2].ambient", 0.0f, 0.0f, 0.0f);
    shader.setVec3("pointLights[2].diffuse", 0.0f, 1.0f, 0.0f);
    shader.setVec3("pointLights[2].specular", 0.0f, 0.5f, 0.0f);
    shader.setFloat("pointLights[2].constant", 1.0f);
    shader.setFloat("pointLights[2].linear", 0.045);
    shader.setFloat("pointLights[2].quadratic", 0.0075);

    shader.setVec3("pointLights[3].ambient", 0.0f, 0.0f, 0.0f);
    shader.setVec3("pointLights[3].diffuse", 0.0f, 0.0f, 1.0f);
    shader.setVec3("pointLights[3].specular", 0.0f, 0.0f, 0.5f);
    shader.setFloat("pointLights[3].constant", 1.0f);
    shader.setFloat("pointLights[3].linear", 0.045);
    shader.setFloat("pointLights[3].quadratic", 0.0075);

    //Model whiteBlock("resources/objects/white-block/white-block.obj");
    glm::vec3 bgColor = glm::vec3(0.5f, 0.5f, 0.5f);
    
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    
    double discoTimeStamp = 0.0f;
    float discoOffset = 0.0f;
    shader.setBool("discoMode", false);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        float currFrameTime = (float) glfwGetTime();
        deltaTime     = currFrameTime - lastFrameTime;
        lastFrameTime = currFrameTime;


        // INPUT
        processInput(window);

        // RENDERING
        glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // Draw
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = camera.getProjectionMatrix();
        //glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)currScrWidth / currScrHeight, 0.1f, 100.0f);
        ////glm::mat4 projection = glm::ortho(-16.0f, 16.0f, -16.0f * currScrHeight / currScrWidth, 16.0f * currScrHeight / currScrWidth, 0.1f, 100.0f);
        //glm::mat4 projection = glm::ortho(-12.0f, 12.0f, -12.0f, 12.0f, 0.1f, 100.0f);

        
        if (game.discoMode || game.discoInitiated)
        {
            if (game.discoInitiated)
            {
                game.discoInitiated = false;
                game.discoMode = true;
                discoTimeStamp = glfwGetTime();
                std::cout << discoTimeStamp << std::endl;

                shader.use();
                shader.setVec3("dirLight.diffuse", 0.0f, 0.0f, 0.0f);
                shader.setVec3("dirLight.specular", 0.0f, 0.0f, 0.0f);
                shader.setBool("discoMode", true);

                for (int i = 0; i < AREA_HEIGHT && game.area.countPerRow[i]; i++)
                    if (game.area.countPerRow[i] > 0)
                        discoOffset = i;
            }
            


            bgColor = glm::vec3(0.2f + sin(glfwGetTime() * 3 - M_PI) / 10,
                                0.2f + sin(glfwGetTime() * 3) / 10,
                                0.2f + sin(glfwGetTime() * 3 + M_PI) / 10);

            float rotSpeed = 1 / 4.0f;
            glm::vec3 pointLightPositions[] = {
                glm::vec3(areaCenter.x + AREA_WIDTH * sin(glfwGetTime() * rotSpeed), areaCenter.y + sin(glfwGetTime()), areaCenter.z + AREA_WIDTH * cos(glfwGetTime() * rotSpeed)),
                glm::vec3(areaCenter.x + AREA_WIDTH * sin(glfwGetTime() * rotSpeed), discoOffset + 3 * sin(glfwGetTime()), areaCenter.z + AREA_WIDTH * cos(glfwGetTime() * rotSpeed)),
                glm::vec3(areaCenter.x + AREA_WIDTH * sin(glfwGetTime() * rotSpeed + 2 * M_PI / 3), discoOffset + 3 * sin(glfwGetTime() * M_PI_2), areaCenter.z + AREA_WIDTH * cos(glfwGetTime() * rotSpeed + 2 * M_PI / 3)),
                glm::vec3(areaCenter.x + AREA_WIDTH * sin(glfwGetTime() * rotSpeed + 4 * M_PI / 3), discoOffset + 3 * sin(glfwGetTime() * M_PI), areaCenter.z + AREA_WIDTH * cos(glfwGetTime() * rotSpeed + 4 * M_PI / 3)),
            };
            shader.use();
            for (int i = 1; i < sizeof(pointLightPositions) / sizeof(glm::vec3); i++)
                shader.setVec3("pointLights[" + std::to_string(i) + "].position", pointLightPositions[i]);
            lightSourceShader.use();
            lightSourceShader.setMat4("projection", projection);
            lightSourceShader.setMat4("view", view);
            for (int i = 1; i < sizeof(pointLightPositions) / sizeof(glm::vec3); i++)
            {
                glm::mat4 model(1.0f);
                model = glm::translate(model, pointLightPositions[i]);
                model = glm::scale(model, glm::vec3(0.4f));
                lightSourceShader.setMat4("model", model);
                block.material = materials[i + 8];
                block.draw();
            }

            // Disco should end
            if (glfwGetTime() - discoTimeStamp > 10.0)
            {
                game.discoMode = false;

                bgColor = glm::vec3(0.5f, 0.5f, 0.5f);

                shader.use();
                shader.setVec3("dirLight.diffuse", 0.8f, 0.8f, 0.8f);
                shader.setVec3("dirLight.specular", 0.3f, 0.3f, 0.3f);
                shader.setBool("discoMode", false);
            }
        }
        

        /*{
        // CUBEMAP
            glm::mat4 model(1.0f);
            model = glm::scale(model, glm::vec3(50.0f));
            shader.setMat4("model", model);

            glBindTexture(GL_TEXTURE_2D, whiteTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }*/

        shader.use();
        shader.setVec3("viewPos", camera.Position);
        
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        game.processLogic();
        game.render(shader);

        // Text
        textShader.use();
        RenderText(textShader, "Score: " + std::to_string(game.score) + (game.discoMode ? "(x3)" : ""), 10.0f, currScrHeight - 48.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        std::stringstream stream;
        stream << std::fixed << std::setprecision(1) << game.speed;
        std::string speed = stream.str();
        RenderText(textShader, "Speed: " + speed, 10.0f, currScrHeight - 72.0f, 0.6f, glm::vec3(0.0f, 0.0f, 0.0f));

        if (game.state == OVER)
        {
            bgColor = glm::vec3(0.8f, 0.0f, 0.0f);
            RenderText(textShader, "Game Over", 100.0f, 300.0f, 2.0f, glm::vec3(1.0f, 1.0f, 1.0f));
            shader.setVec3("dirLight.ambient", 0.05f, 0.0f, 0.0f);
            shader.setVec3("dirLight.diffuse", 0.8f, 0.0f, 0.0f);
        }

        // Check and call events and swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // izbrisat buffere??
        
    shader.del();
    block.del();
    //whiteBlock.del();

    
    glfwTerminate();
    return 0;
}

// glfw: whenever the window size changes (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    currScrWidth = width;
    currScrHeight = height;
}

bool pressedT = false, pressedR = false;
bool spacePressed = false;
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.processKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.processKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.processKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.processKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.processKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.processKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !pressedT) {
        camera.toggleMode();
        pressedT = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE)
        pressedT = false;


    // Handle block control
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE
            && glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE
            && glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE)
        game.transform(NONE);

    if ((glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && camera.Yaw >= 225 && camera.Yaw < 315)
            || (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && camera.Yaw >= 135 && camera.Yaw < 225)
            || (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && camera.Yaw >= 45 && camera.Yaw < 135)
            || (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && (camera.Yaw >= 315 || camera.Yaw < 45)))
        game.transform(TRANS_RIGHT);
    if ((glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && camera.Yaw >= 225 && camera.Yaw < 315)
            || (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && camera.Yaw >= 135 && camera.Yaw < 225)
            || (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && camera.Yaw >= 45 && camera.Yaw < 135)
            || (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && (camera.Yaw >= 315 || camera.Yaw < 45)))
        game.transform(TRANS_LEFT);

    if ((glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && camera.Yaw >= 225 && camera.Yaw < 315)
            || (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && camera.Yaw >= 135 && camera.Yaw < 225)
            || (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && camera.Yaw >= 45 && camera.Yaw < 135)
            || (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && (camera.Yaw >= 315 || camera.Yaw < 45)))
        game.transform(TRANS_BACKWARD);
    if ((glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && camera.Yaw >= 225 && camera.Yaw < 315)
            || (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && camera.Yaw >= 135 && camera.Yaw < 225)
            || (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && camera.Yaw >= 45 && camera.Yaw < 135)
            || (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && (camera.Yaw >= 315 || camera.Yaw < 45)))
        game.transform(TRANS_FORWARD);
    
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        game.transform(ROT_CW);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        game.transform(ROT_CCW);

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !pressedR)
    {
        pressedR = true;
        if (game.player.rotationAxis == AXIS_X)
            game.setRotationAxis(AXIS_Y);
        else if (game.player.rotationAxis == AXIS_Y)
            game.setRotationAxis(AXIS_Z);
        else
            game.setRotationAxis(AXIS_X);
    }
    else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
        pressedR = false;
        
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed)
    {
        game.drop();
        spacePressed = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
        spacePressed = false;

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        if (game.state == ACTIVE) game.state = PAUSED;
        else if (game.state == PAUSED) game.state = ACTIVE;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{           
    /* if (button = GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        std::cout << "rotation click" << std::endl;
        if (game.player.rotationAxis == AXIS_X)
            game.setRotationAxis(AXIS_Y);
        else if (game.player.rotationAxis == AXIS_Y)
            game.setRotationAxis(AXIS_Z);
        else
            game.setRotationAxis(AXIS_X);
    } */
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

    //camera.processMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    //camera.processMouseScroll((float) yoffset);
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
        
        glActiveTexture(GL_TEXTURE0);
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

void initFreeType()
{
    // FONT
    FT_Library ftl;
    if (FT_Init_FreeType(&ftl))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }
    FT_Face face;
    if (FT_New_Face(ftl, "resources/fonts/joystix/joystix-monospace.otf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;  
        return;
    }
    
    FT_Set_Pixel_Sizes(face, 0, 32);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
  
    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // now store character for later use
        Character character = {
            texture, 
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        characters.insert(std::pair<char, Character>(c, character));
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ftl);

    // configure VAO/VBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &fVAO);
    glGenBuffers(1, &fVBO);
    glBindVertexArray(fVAO);
    glBindBuffer(GL_ARRAY_BUFFER, fVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
    // activate corresponding render state	
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(fVAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = characters[*c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, fVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
