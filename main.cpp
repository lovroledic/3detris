#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "shader.hpp"
#include "camera_ortho.hpp"
#include "model.hpp"
#include "block.hpp"

//#include "game.hpp"
#include "game_logic.hpp"

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

void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color);


const unsigned int SCREEN_WIDTH  = 1200;
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
    camera = Camera(
        glm::vec3(game.area.WIDTH_X / 2.0f - 0.5f, game.area.HEIGHT, game.area.WIDTH_Z / 2.0f - 0.5f),
        glm::vec3(-1.0f, 0.0f, 0.0f), game.area.WIDTH_X / sqrt(2),
        90, -45
    );
    block = Block("resources/objects/white-block/white-block.obj", 0);

    // Build and compile shader program
    Shader shader("shaders/my_shader.vert", "shaders/my_shader.frag");
    Shader textShader("shaders/text.vert", "shaders/text.frag");
    
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(currScrWidth), 0.0f, static_cast<float>(currScrHeight));
    textShader.use();
    textShader.setMat4("projection", projection);

    
    textures.push_back(loadTexture("resources/textures/colors/white.png"));
    textures.push_back(loadTexture("resources/textures/colors/red.png"));
    textures.push_back(loadTexture("resources/textures/colors/green.png"));
    textures.push_back(loadTexture("resources/textures/colors/blue.png"));
    textures.push_back(loadTexture("resources/textures/colors/cyan.png"));
    textures.push_back(loadTexture("resources/textures/colors/magenta.png"));
    textures.push_back(loadTexture("resources/textures/colors/yellow.png"));


    shader.use();
    shader.setFloat("material.shininess", 100.0f);
    shader.setVec3("dirLight.direction", 0.5f, 1.0f, 0.3f);
    shader.setVec3("dirLight.ambient", 0.3f, 0.3f, 0.3f);
    shader.setVec3("dirLight.diffuse", 0.6f, 0.6f, 0.6f);
    shader.setVec3("dirLight.specular", 0.0f, 0.0f, 0.0f);

    Model whiteBlock("resources/objects/white-block/white-block.obj");
    glm::vec3 bgColor = glm::vec3(0.6f, 0.6f, 0.6f);

    // FONT
    FT_Library ftl;
    if (FT_Init_FreeType(&ftl))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }
    FT_Face face;
    if (FT_New_Face(ftl, "resources/fonts/LiberationMono-Regular.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;  
        return -1;
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
    
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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


        // Text
        textShader.use();
        RenderText(textShader, "Score: " + std::to_string(game.score), 10.0f, currScrHeight - 48.0f, 1.0f, glm::vec3(0.6f, 0.3f, 1.0f));
        if (game.state == OVER)
            RenderText(textShader, "Game Over", 10.0f, 10.0f, 2.0f, glm::vec3(0.8f, 0.0f, 0.0f));


        // Draw
        shader.use();
        shader.setVec3("viewPos", camera.Position);

        glm::mat4 view = camera.getViewMatrix();
        //glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)currScrWidth / currScrHeight, 0.1f, 100.0f);
        glm::mat4 projection = glm::ortho(-16.0f, 16.0f, -16.0f * currScrHeight / currScrWidth, 16.0f * currScrHeight / currScrWidth, 0.1f, 100.0f);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
    

        glm::mat4 model(1.0f);
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

        game.processLogic();
        game.render(shader);

        //std::cout << game.player.offset.x << "\t" << game.player.offset.y << "\t" << game.player.offset.z << std::endl;

        // Check and call events and swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // izbrisat buffere??
        
    shader.del();
    block.del();
    whiteBlock.del();


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
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE
            && glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE
            && glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE)
        game.transform(NONE);

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        game.transform(TRANS_RIGHT);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        game.transform(TRANS_LEFT);

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        game.transform(TRANS_BACKWARD);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        game.transform(TRANS_FORWARD);
    
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        game.transform(ROT_CW);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        game.transform(ROT_CCW);

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && game.state == ACTIVE)
        game.state = PAUSED;


}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{   
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        game.drop();
    }
        
    else if (button = GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        std::cout << "rotation click" << std::endl;
        if (game.player.rotationAxis == AXIS_X)
            game.setRotationAxis(AXIS_Y);
        else if (game.player.rotationAxis == AXIS_Y)
            game.setRotationAxis(AXIS_Z);
        else
            game.setRotationAxis(AXIS_X);
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
