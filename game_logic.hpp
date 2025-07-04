#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <map>

#include "block.hpp"
#include "shape.hpp"
#include "shader.hpp"

#include "constants.hpp"


Block block;


// PLAYER

class Player
{
    public:
        Shape shape;
        int materialIndex;
        glm::ivec3 prevOffset;
        glm::ivec3 offset; // offset within Area
        Axis rotationAxis = AXIS_Y;

        void setShape(int index) { shape = shapes[index]; }
        void setMaterial(int matIndex) { materialIndex = matIndex; };

        void render(Shader &, bool);
        void renderPreview(Shader &, glm::vec3, bool, int); 
};

void Player::render(Shader &shader, bool discoMode)
{
    block.material = discoMode ? materials[0] : materials[materialIndex - 1];
    shader.setFloat("material.shininess", block.material.Ns);
    
    for (int i = 0; i < SHAPE_WIDTH; i++)
        for (int j = 0; j < SHAPE_WIDTH; j++)
            for (int k = 0; k < SHAPE_WIDTH; k++)
                if (shape.positions[i][j][k])
                {
                    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(offset.x + i, offset.y + j, offset.z + k));
                    shader.setMat4("model", model);
                    block.draw();
                }
}

// Render a preview of where the block would be positioned if it were dropped
void Player::renderPreview(Shader &shader, glm::vec3 cameraPos, bool discoMode, int offsetY)
{
    // Player is already positioned where it can drop the lowest
    if (offsetY == 0)
        return;
    
    // Sort preview block positions from closest to furthest from camera
    // NOTE: Generally, the preferred way of rendering transparent objects is by first rendering the furthest objects so you can see the overlapping objects.
    // However, with the preview we want only the closest blocks to be visible so it looks the same as if weren't transparent at all (same as Player::render),
    // while still being able to see other (static) blocks behind the preview.
    std::map<float, glm::vec3> sortedPositions;
    for (int i = 0; i < SHAPE_WIDTH; i++)
        for (int j = 0; j < SHAPE_WIDTH; j++)
            for (int k = 0; k < SHAPE_WIDTH; k++)
                if (shape.positions[i][j][k])
                {
                    glm::vec3 previewBlockPos = glm::vec3(offset.x + i, offset.y + j - offsetY, offset.z + k);
                    float dist = glm::length(previewBlockPos - cameraPos);
                    sortedPositions[dist] = previewBlockPos;
                }
            
    // Rendering
    block.material = discoMode ? materials[0] : materials[materialIndex - 1];
    shader.setFloat("material.shininess", block.material.Ns);
    shader.setFloat("alpha", 0.4f + sin(glfwGetTime() * M_PI) / 4.0f);
    for (std::map<float, glm::vec3>::iterator it = sortedPositions.begin(); it != sortedPositions.end(); it++)
    {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), it->second);
        shader.setMat4("model", model);
        block.draw();
    }
    shader.setFloat("alpha", 1.0f);
}


// AREA
#define AREA_WIDTH 7
#define AREA_HEIGHT 12

class Area
{
    public:
        static const int WIDTH = AREA_WIDTH;
        static const int HEIGHT = AREA_HEIGHT;
        int positions[WIDTH][HEIGHT][WIDTH] = { 0 };
        int countPerRow[HEIGHT];


        void init() { initBorder(); }
        void renderBorder(Shader &);
        void renderStaticBlocks(Shader &, bool);
        glm::vec3 getCenter() { return glm::vec3(WIDTH / 2.0f - 0.5f, HEIGHT / 2.0f - 0.5f, WIDTH / 2.0f - 0.5f); }

    private:
        GLuint borderVBO, borderVAO;
        
        void initBorder();
};

void Area::renderBorder(Shader &shader)
{
    glBindVertexArray(borderVAO);
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    glDrawArrays(GL_LINES, 0, 24);
}

void Area::renderStaticBlocks(Shader &shader, bool discoMode)
{
    glm::mat4 model;
    for (int i = 0; i < WIDTH; i++)
        for (int j = 0; j < HEIGHT; j++)
            for (int k = 0; k < WIDTH; k++)
                if (positions[i][j][k])
                {
                    model = glm::translate(glm::mat4(1.0f), glm::vec3(i, j, k));
                    shader.setMat4("model", model);
                    block.material = discoMode ? materials[0] : materials[positions[i][j][k] - 1];
                    shader.setFloat("material.shininess", block.material.Ns);
                    block.draw();
                }
}

// Initialize OpenGL buffers for rendering border
void Area::initBorder()
{
    float borderVertices[] = {
        // bottom square
                - 0.5f,         - 0.5f,          - 0.5f,
                - 0.5f,         - 0.5f,  WIDTH - 0.5f,
                - 0.5f,         - 0.5f,  WIDTH - 0.5f,
        WIDTH - 0.5f,         - 0.5f,  WIDTH - 0.5f,
        WIDTH - 0.5f,         - 0.5f,  WIDTH - 0.5f,
        WIDTH - 0.5f,         - 0.5f,          - 0.5f,
        WIDTH - 0.5f,         - 0.5f,          - 0.5f,
                - 0.5f,         - 0.5f,          - 0.5f,
        // top square 
                - 0.5f,  HEIGHT - 0.5f,          - 0.5f,
                - 0.5f,  HEIGHT - 0.5f,  WIDTH - 0.5f,
                - 0.5f,  HEIGHT - 0.5f,  WIDTH - 0.5f,
        WIDTH - 0.5f,  HEIGHT - 0.5f,  WIDTH - 0.5f,
        WIDTH - 0.5f,  HEIGHT - 0.5f,  WIDTH - 0.5f,
        WIDTH - 0.5f,  HEIGHT - 0.5f,          - 0.5f,
        WIDTH - 0.5f,  HEIGHT - 0.5f,          - 0.5f,
                - 0.5f,  HEIGHT - 0.5f,          - 0.5f,
        // vertical lines connecting bottom and top square
                - 0.5f,         - 0.5f,          - 0.5f,
                - 0.5f,  HEIGHT - 0.5f,          - 0.5f,
                - 0.5f,         - 0.5f,  WIDTH - 0.5f,
                - 0.5f,  HEIGHT - 0.5f,  WIDTH - 0.5f,
        WIDTH - 0.5f,         - 0.5f,  WIDTH - 0.5f,
        WIDTH - 0.5f,  HEIGHT - 0.5f,  WIDTH - 0.5f,
        WIDTH - 0.5f,         - 0.5f,          - 0.5f,
        WIDTH - 0.5f,  HEIGHT - 0.5f,          - 0.5f,
    };

    glGenBuffers(1, &borderVBO);
    glGenVertexArrays(1, &borderVAO);

    glBindBuffer(GL_ARRAY_BUFFER, borderVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(borderVertices), borderVertices, GL_STATIC_DRAW);

    glBindVertexArray(borderVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
}



int getPreviewOffset(Player, Area);

// GAME

enum State {
    ACTIVE,
    PAUSED,
    OVER
};

class Game
{
    public:
        State state;
        double speed = 1.0f;
        int score = 0;
        bool discoMode = false;
        bool discoInitiated = false;

        Player player;
        Area area;


        void init();
        void processLogic();
        void render(Shader &shader, glm::vec3 cameraPos);

        void transform(Transformation);
        void drop();

        void setRotationAxis(Axis);

    private:
        int tick = 0;
        double tickOffset = 0;
        double tickDropOffset = 0;
        int dropOffset = 0;
        int initLowestIndex = 0;

        bool keyPressed = false;
        bool shouldSpawnNewBlock = true;
        bool collisionDetected = false;
        bool dropInitiated = false;

        bool checkCollision(Shape);
        bool detectHorizontalCollision(Shape);

        GLuint xAxisVBO, xAxisVAO;
        GLuint yAxisVBO, yAxisVAO;
        GLuint zAxisVBO, zAxisVAO;
        GLuint currAxisVAO;

        void initRotationAxis();
        void renderRotationAxis(Shader &);
};

void Game::init()
{
    initRotationAxis();
    area.init();
}

void Game::processLogic()
{
    if (state != ACTIVE)
        return;
    // TODO: posebna funkcija za procesiranje akcija kad je igra pauzirana


    int &pox = player.offset.x;
    int &poy = player.offset.y;
    int &poz = player.offset.z;

    // Update Player shape if 1) game started, or 2) new "level" started
    if (shouldSpawnNewBlock)
    {
        int r = rand() % 8;//(sizeof(shapes) / sizeof(Shape));
        std::cout << "Rand " << r << std::endl;
        player.setShape(r);
        player.setMaterial(r + 1);

        // Randomize initial rotation
        int rx, ry, rz;
        rx = rand() % 4;
        ry = rand() % 4;
        rz = rand() % 4;
        for (int i = 0; i < rx; i++)
            player.shape.rotate(AXIS_X, ROT_CW);
        for (int i = 0; i < ry; i++)
            player.shape.rotate(AXIS_Y, ROT_CW);
        for (int i = 0; i < rz; i++)
            player.shape.rotate(AXIS_Z, ROT_CW);

        pox = (area.WIDTH - SHAPE_WIDTH) / 2;
        poz = (area.WIDTH - SHAPE_WIDTH) / 2;

        shouldSpawnNewBlock = false;
        dropOffset = 0;
        initLowestIndex = player.shape.getLowestIndex(); // to prevent updates when rotating
    }

    // Tick logic
    tick = glfwGetTime() * 1.25f * speed - tickOffset - tickDropOffset; // BUG: treba ažurirat tickOffset kad se pauzira

    
    

    poy = area.HEIGHT - initLowestIndex - tick - dropOffset;


    // Prevent unnecessary calculations if Player didn't move
    if (player.prevOffset == player.offset)
        return;
    
    player.prevOffset = player.offset;
   
    
    // If no collision was detected, move to next frame
    collisionDetected = checkCollision(player.shape);
    if (!collisionDetected)
        return;
    
    // Collision detected
    
    // Check if locking Player shape in place would cause Game Over
    for (int j = 0; j < SHAPE_WIDTH; j++)
    {
        for (int i = 0; i < SHAPE_WIDTH; i++)
            for (int k = 0; k < SHAPE_WIDTH; k++)
                if (player.shape.positions[i][j][k] && poy + j == area.HEIGHT - 1)
                {
                    std::cout << "over" << std::endl;
                    state = OVER;
                    poy++; // This way the player is rendered above the collision
                    return;
                }
    }

    // Lock the Player in place
    for (int j = 0; j < SHAPE_WIDTH; j++)
    {
        for (int i = 0; i < SHAPE_WIDTH; i++)
            for (int k = 0; k < SHAPE_WIDTH; k++)
                if (player.shape.positions[i][j][k])
                {
                    area.countPerRow[player.offset.y + j + 1]++; // (j + 1) since collision happened at (j)
                    area.positions[player.offset.x + i][player.offset.y + j + 1][player.offset.z + k] = player.materialIndex;
                    if (player.materialIndex == 0)
                    {
                        std::cout << "Yup " << area.positions[player.offset.x + i][player.offset.y + j + 1][player.offset.z + k] << std::endl;
                    }
                }
    }
    
    // Scoring
    score += discoMode ? player.shape.count * 3 : player.shape.count;

    // Check if any rows got filled up; if so, clear them
    // OPTIMIZE: trenutno je brute force
    bool speedIncreased = false;
    int rowsCleared = 0;
    for (int j = area.HEIGHT - 1; j >= 0; j--)
    {
        if (area.countPerRow[j] == area.WIDTH * area.WIDTH) // Row is filled
        {
            // Increase speed only once, no matter how many rows were cleared
            if (!speedIncreased)
            {
                speed += 0.1;
            }
            speedIncreased = true;

            // Bring everything down by 1
            for (int jj = j; jj < area.HEIGHT - 2; jj++)
            {
                area.countPerRow[jj] = area.countPerRow[jj + 1];

                for (int i = 0; i < area.WIDTH; i++)
                    for (int k = 0; k < area.WIDTH; k++)
                        area.positions[i][jj][k] = area.positions[i][jj + 1][k];
            }

            // Clear top row
            // FIXME: nema potrebe čistit gornji red svaki put kad se očisti red, samo za jedan je potrebno
            for (int i = 0; i < area.WIDTH; i++)
                for (int k = 0; k < area.WIDTH; k++)
                    area.positions[i][area.HEIGHT - 1][k] = 0;

            rowsCleared++;
        }
    }
    if (rowsCleared >= 1)
    {
        discoMode = true;
        discoInitiated = true;
        std::cout << "Start disco!!!" << std::endl;
    }



    std::cout << "Level ended at:\t" << glfwGetTime() * 1.25f * speed - tickOffset << std::endl;
    // Signal new level
    dropOffset = 0;
    tickOffset = glfwGetTime() * 1.25f * speed;
    tickDropOffset = 0.0;
    shouldSpawnNewBlock = true;
}

void Game::render(Shader &shader, glm::vec3 cameraPos)
{
    shader.use();

    area.renderBorder(shader);

    if (!collisionDetected || state == OVER)
    {
        player.render(shader, discoMode);
    }

    if (state != OVER)
        renderRotationAxis(shader);

    area.renderStaticBlocks(shader, discoMode); // Must be called after rendering Player block to prevent visual stutter
    
    int offsetY = getPreviewOffset(player, area); // OPTIMIZE: pozvati samo kad se player pomakne: 1) započeo novi tick, 2) transform(), 3) drop
    player.renderPreview(shader, cameraPos, discoMode, offsetY);
}

void Game::transform(Transformation transform)
{
    if (state != ACTIVE)
        return;

    if (transform == NONE)
    {
        keyPressed = false;
        return;
    }

    if (keyPressed)
        return;

    int &pox = player.offset.x;
    int &poy = player.offset.y;
    int &poz = player.offset.z;


    if (transform == TRANS_BACKWARD)
    {
        keyPressed = true;

        // Check for collision with border...
        for (int i = 0; i < SHAPE_WIDTH; i++)
            for (int j = 0; j < SHAPE_WIDTH; j++)
                for (int k = 0; k < SHAPE_WIDTH; k++)
                    if (player.shape.positions[i][j][k] && player.offset.z + k - 1 < 0)
                        return;

        // ... or with other static blocks
        for (int i = 0; i < SHAPE_WIDTH; i++)
            for (int j = 0; j < SHAPE_WIDTH; j++)
                for (int k = 0; k < SHAPE_WIDTH; k++)
                    if (poy + j < area.HEIGHT && player.shape.positions[i][j][k] && area.positions[pox + i][poy + j][poz + k - 1])
                        return;

        player.offset.z -= 1; // forward is in the direction of -z
    }
    else if (transform == TRANS_FORWARD)
    {
        keyPressed = true;

        // Check for collision with border...
        for (int i = 0; i < SHAPE_WIDTH; i++)
            for (int j = 0; j < SHAPE_WIDTH; j++)
                for (int k = 0; k < SHAPE_WIDTH; k++)
                    if (player.shape.positions[i][j][k] && player.offset.z + k + 1 >= area.WIDTH)
                        return;

        // ... or with other static blocks
        for (int i = 0; i < SHAPE_WIDTH; i++)
            for (int j = 0; j < SHAPE_WIDTH; j++)
                for (int k = 0; k < SHAPE_WIDTH; k++)
                    if (poy + j < area.HEIGHT && player.shape.positions[i][j][k] && area.positions[pox + i][poy + j][poz + k + 1])
                        return;
        
        player.offset.z += 1; // backward is in the direction of +z
    }
    else if (transform == TRANS_RIGHT)
    {
        keyPressed = true;

        // Check for collision with border...
        for (int i = 0; i < SHAPE_WIDTH; i++)
            for (int j = 0; j < SHAPE_WIDTH; j++)
                for (int k = 0; k < SHAPE_WIDTH; k++)
                    if (player.shape.positions[i][j][k] && player.offset.x + i - 1 < 0)
                        return;

        // ... or with other static blocks
        for (int i = 0; i < SHAPE_WIDTH; i++)
            for (int j = 0; j < SHAPE_WIDTH; j++)
                for (int k = 0; k < SHAPE_WIDTH; k++)
                    if (poy + j < area.HEIGHT && player.shape.positions[i][j][k] && area.positions[pox + i - 1][poy + j][poz + k])
                        return;
        
        player.offset.x -= 1; // backward is in the direction of +z
    }
    else if (transform == TRANS_LEFT)
    {
        keyPressed = true;

        // Check for collision with border...
        for (int i = 0; i < SHAPE_WIDTH; i++)
            for (int j = 0; j < SHAPE_WIDTH; j++)
                for (int k = 0; k < SHAPE_WIDTH; k++)
                    if (player.shape.positions[i][j][k] && player.offset.x + i + 1 >= area.WIDTH)
                        return;

        // ... or with other static blocks
        for (int i = 0; i < SHAPE_WIDTH; i++)
            for (int j = 0; j < SHAPE_WIDTH; j++)
                for (int k = 0; k < SHAPE_WIDTH; k++)
                    if (poy + j < area.HEIGHT && player.shape.positions[i][j][k] && area.positions[pox + i + 1][poy + j][poz + k])
                        return;

        player.offset.x += 1; // backward is in the direction of +z
    }

    else if (transform == ROT_CCW)
    {
        keyPressed = true;
        Shape rotated = player.shape.getRotated(player.rotationAxis, ROT_CCW);
        if (!detectHorizontalCollision(rotated))
            player.shape = rotated;
    }
    else if (transform == ROT_CW)
    {
        keyPressed = true;
        Shape rotated = player.shape.getRotated(player.rotationAxis, ROT_CW);
        if (!detectHorizontalCollision(rotated))
            player.shape = rotated;
    }
}

void Game::drop()
{
    dropOffset += getPreviewOffset(player, area);
    tickDropOffset = glfwGetTime() * 1.25f * speed - tickOffset - (int)(glfwGetTime() * 1.25f * speed - tickOffset);//glfwGetTime() * speed - (int)(glfwGetTime() * speed);
}

void Game::setRotationAxis(Axis newAxis)
{
    player.rotationAxis = newAxis;
    
    if (newAxis == AXIS_X)
        currAxisVAO = xAxisVAO;
    else if (newAxis == AXIS_Y)
        currAxisVAO = yAxisVAO;
    else
        currAxisVAO = zAxisVAO;
}

// REVIEW: preimenuj u detect*Vertical*Collision
bool Game::checkCollision(Shape shape)
{
    int pox = player.offset.x;
    int poy = player.offset.y;
    int poz = player.offset.z;

    // Check if Player collided with ground
    for (int i = 0; i < SHAPE_WIDTH; i++)
        for (int j = 0; j < SHAPE_WIDTH; j++)
            for (int k = 0; k < SHAPE_WIDTH; k++)
                if (shape.positions[i][j][k] && player.offset.y + j < 0)
                {
                    std::cout << "Collision with ground at " << pox + i << ", " << poy + j << ", " << poz + k << std::endl;
                    return true;
                }

    // Check for collision with static block
    for (int i = 0; i < SHAPE_WIDTH; i++)
        for (int j = 0; j < SHAPE_WIDTH; j++)
            for (int k = 0; k < SHAPE_WIDTH; k++)
                if (poy + j < area.HEIGHT && shape.positions[i][j][k] && area.positions[pox + i][poy + j][poz + k]) // can't use == in case both are false
                {
                    std::cout << "Collision with block at " << pox + i << ", " << poy + j << ", " << poz + k << std::endl;
                    return true;
                }

    return false;
}

bool Game::detectHorizontalCollision(Shape shape)
{
    int pox = player.offset.x;
    int poy = player.offset.y;
    int poz = player.offset.z;

    // Check for collision with static block
    for (int i = 0; i < SHAPE_WIDTH; i++)
        for (int j = 0; j < SHAPE_WIDTH; j++)
            for (int k = 0; k < SHAPE_WIDTH; k++)
                if (poy + j < area.HEIGHT && shape.positions[i][j][k] && area.positions[pox + i][poy + j][poz + k])
                    return true;
    
    // Check for collision with side border (relevant for rotations)
    for (int i = 0; i < SHAPE_WIDTH; i++)
        for (int j = 0; j < SHAPE_WIDTH; j++)
            for (int k = 0; k < SHAPE_WIDTH; k++)
                if (shape.positions[i][j][k] &&
                        (pox + i < 0 || pox + i >= area.WIDTH || // pox + i + 1 >= area.WIDTH
                         poz + k < 0 || poz + k >= area.WIDTH))
                    return true;

    return false;
}

// Initialize OpenGL buffers for rendering rotation axis
void Game::initRotationAxis()
{
    float xAxisVertices[] = {
        0, 0, 0,
        1, 0, 0,
    };
    float yAxisVertices[] = {
        0, 0, 0,
        0, 1, 0,
    };
    float zAxisVertices[] = {
        0, 0, 0,
        0, 0, 1,
    };

    // x-axis
    glGenBuffers(1, &xAxisVBO);
    glGenVertexArrays(1, &xAxisVAO);

    glBindBuffer(GL_ARRAY_BUFFER, xAxisVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(xAxisVertices), xAxisVertices, GL_STATIC_DRAW);

    glBindVertexArray(xAxisVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // y-axis
    glGenBuffers(1, &yAxisVBO);
    glGenVertexArrays(1, &yAxisVAO);

    glBindBuffer(GL_ARRAY_BUFFER, yAxisVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(yAxisVertices), yAxisVertices, GL_STATIC_DRAW);

    glBindVertexArray(yAxisVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // z-axis
    glGenBuffers(1, &zAxisVBO);
    glGenVertexArrays(1, &zAxisVAO);

    glBindBuffer(GL_ARRAY_BUFFER, zAxisVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(zAxisVertices), zAxisVertices, GL_STATIC_DRAW);

    glBindVertexArray(zAxisVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    currAxisVAO = yAxisVAO;
}

void Game::renderRotationAxis(Shader &shader)
{
    glm::mat4 model = glm::mat4(1.0f);

    // x-axis
    if (player.rotationAxis == AXIS_X)
    {
        model = glm::translate(model, glm::vec3(-0.5f, player.offset.y + SHAPE_WIDTH / 2.0f - 0.5f, player.offset.z + SHAPE_WIDTH / 2.0f - 0.5f));
        model = glm::scale(model, glm::vec3(area.WIDTH));
    }

    // y-axis
    else if (player.rotationAxis == AXIS_Y)
    {
        model = glm::translate(model, glm::vec3(player.offset.x + SHAPE_WIDTH / 2.0f - 0.5f, -0.5f, player.offset.z + SHAPE_WIDTH / 2.0f - 0.5f));
        model = glm::scale(model, glm::vec3(area.HEIGHT));
    }

    // z-axis
    else
    {
        model = glm::translate(model, glm::vec3(player.offset.x + SHAPE_WIDTH / 2.0f - 0.5f, player.offset.y + SHAPE_WIDTH / 2.0f - 0.5f, -0.5f));
        model = glm::scale(model, glm::vec3(area.WIDTH));
    }
    
    glBindVertexArray(currAxisVAO);
    shader.setMat4("model", model);
    glDrawArrays(GL_LINES, 0, 2);
}


int getPreviewOffset(Player player, Area area)
{
    int &pox = player.offset.x;
    int &poy = player.offset.y;
    int &poz = player.offset.z;
    int li = player.shape.getLowestIndex();

    for (int aj = poy; aj >= -SHAPE_WIDTH + 1; aj--)
    {
        for (int j = li; j < SHAPE_WIDTH; j++)
            for (int i = 0; i < SHAPE_WIDTH; i++)
                for (int k = 0; k < SHAPE_WIDTH; k++)
                    // Detect collision
                    if (player.shape.positions[i][j][k]
                            && (aj + j < 0 || (aj + j < AREA_HEIGHT && area.positions[pox + i][aj + j][poz + k])))
                        return poy - (aj + 1); // a + j = position above collision
    }
}

#endif