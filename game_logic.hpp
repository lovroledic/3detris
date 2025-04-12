#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>

#include "block.hpp"
#include "shape.hpp"
#include "shader.hpp"

#include "constants.hpp"


Block block;
std::vector<GLuint> textures;


// PLAYER

class Player
{
    public:
        Shape shape;
        GLuint texture;
        glm::ivec3 offset; // offset within Area
        Axis rotationAxis = AXIS_Y;


        void setShape(int index) { shape = shapes[index]; }
        void setColor(GLuint texID) { texture = texID; }

        void render(Shader &);

    private:
        
};

void Player::render(Shader &shader)
{
    glm::mat4 model;
    for (int i = 0; i < SHAPE_WIDTH; i++)
        for (int j = 0; j < SHAPE_WIDTH; j++)
            for (int k = 0; k < SHAPE_WIDTH; k++)
                if (shape.positions[i][j][k])
                {
                    model = glm::translate(glm::mat4(1.0f), glm::vec3(offset.x + i, offset.y + j, offset.z + k));
                    shader.setMat4("model", model);
                    block.textureID = texture;
                    block.draw();
                }
}


// AREA

class Area
{
    public:
        static const int WIDTH_X = 8, HEIGHT = 16, WIDTH_Z = 8;
        GLuint positions[WIDTH_X][HEIGHT][WIDTH_Z] = { 0 };
        int countPerRow[HEIGHT];


        void init() { initBorder(); }
        void renderBorder(Shader &);
        void renderStaticBlocks(Shader &);

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

void Area::renderStaticBlocks(Shader &shader)
{
    glm::mat4 model;
    for (int i = 0; i < WIDTH_X; i++)
        for (int j = 0; j < HEIGHT; j++)
            for (int k = 0; k < WIDTH_Z; k++)
                if (positions[i][j][k])
                {
                    model = glm::translate(glm::mat4(1.0f), glm::vec3(i, j, k));
                    shader.setMat4("model", model);
                    block.textureID = positions[i][j][k];
                    block.draw();
                }
}

// Initialize OpenGL buffers for rendering border
void Area::initBorder()
{
    float borderVertices[] = {
        // bottom square
                - 0.5f,         - 0.5f,          - 0.5f,
                - 0.5f,         - 0.5f,  WIDTH_Z - 0.5f,
                - 0.5f,         - 0.5f,  WIDTH_Z - 0.5f,
        WIDTH_X - 0.5f,         - 0.5f,  WIDTH_Z - 0.5f,
        WIDTH_X - 0.5f,         - 0.5f,  WIDTH_Z - 0.5f,
        WIDTH_X - 0.5f,         - 0.5f,          - 0.5f,
        WIDTH_X - 0.5f,         - 0.5f,          - 0.5f,
                - 0.5f,         - 0.5f,          - 0.5f,
        // top square 
                - 0.5f,  HEIGHT - 0.5f,          - 0.5f,
                - 0.5f,  HEIGHT - 0.5f,  WIDTH_Z - 0.5f,
                - 0.5f,  HEIGHT - 0.5f,  WIDTH_Z - 0.5f,
        WIDTH_X - 0.5f,  HEIGHT - 0.5f,  WIDTH_Z - 0.5f,
        WIDTH_X - 0.5f,  HEIGHT - 0.5f,  WIDTH_Z - 0.5f,
        WIDTH_X - 0.5f,  HEIGHT - 0.5f,          - 0.5f,
        WIDTH_X - 0.5f,  HEIGHT - 0.5f,          - 0.5f,
                - 0.5f,  HEIGHT - 0.5f,          - 0.5f,
        // vertical lines connecting bottom and top square
                - 0.5f,         - 0.5f,          - 0.5f,
                - 0.5f,  HEIGHT - 0.5f,          - 0.5f,
                - 0.5f,         - 0.5f,  WIDTH_Z - 0.5f,
                - 0.5f,  HEIGHT - 0.5f,  WIDTH_Z - 0.5f,
        WIDTH_X - 0.5f,         - 0.5f,  WIDTH_Z - 0.5f,
        WIDTH_X - 0.5f,  HEIGHT - 0.5f,  WIDTH_Z - 0.5f,
        WIDTH_X - 0.5f,         - 0.5f,          - 0.5f,
        WIDTH_X - 0.5f,  HEIGHT - 0.5f,          - 0.5f,
    };

    glGenBuffers(1, &borderVBO);
    glGenVertexArrays(1, &borderVAO);

    glBindBuffer(GL_ARRAY_BUFFER, borderVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(borderVertices), borderVertices, GL_STATIC_DRAW);

    glBindVertexArray(borderVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
}


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

        Player player;
        Area area;


        void init();
        void processLogic();
        void render(Shader &shader);

        void transform(Transformation);
        void drop();

        void setRotationAxis(Axis);

    private:
        //bool newTick = true; // game starts on new tick
        //int prevTick = 0;
        //double newLevelTickOffset = 0; // Brings 'tick' down to 0 when next block spawns
        int tick = 0;
        double tickOffset = 0;
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

    // Tick logic
    tick = glfwGetTime() * speed - tickOffset;
    /* tick = glfwGetTime() * speed - newLevelTickOffset;
    if (tick != prevTick)
    {
        newTick = true;
        prevTick = tick;
        /* dropInitiated = false;
        newLevelTickOffset = glfwGetTime() * speed;
    }
    if (!newTick)
        return;
    newTick = false; */


    int &pox = player.offset.x;
    int &poy = player.offset.y;
    int &poz = player.offset.z;
    
    // Update Player shape if 1) game started, or 2) new "level" started
    if (shouldSpawnNewBlock)
    {
        int rs = rand() % (sizeof(shapes) / sizeof(Shape));
        player.setShape(rs);

        //int rc = rand() % textures.size();
        player.setColor(textures[rs]);


        pox = (area.WIDTH_X - SHAPE_WIDTH) / 2;
        poz = (area.WIDTH_Z - SHAPE_WIDTH) / 2;

        shouldSpawnNewBlock = false;
        dropOffset = 0;
        initLowestIndex = player.shape.getLowestIndex(); // to prevent updates when rotating
        //poy = area.HEIGHT - player.shape.getLowestIndex();
    }
    //else
        //poy--;

    poy = area.HEIGHT - initLowestIndex - tick - dropOffset;
        
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
                    return;
                }
    }


    // Scoring
    score += player.shape.count;

    // Lock the Player in place
    for (int j = 0; j < SHAPE_WIDTH; j++)
    {
        for (int i = 0; i < SHAPE_WIDTH; i++)
            for (int k = 0; k < SHAPE_WIDTH; k++)
                if (player.shape.positions[i][j][k])
                {
                    area.countPerRow[player.offset.y + j + 1]++; // (j + 1) since collision happened at (j)
                    area.positions[player.offset.x + i][player.offset.y + j + 1][player.offset.z + k] = player.texture;
                }
    }
    

    // Check if any rows got filled up; if so, clear them
    // OPTIMIZE: trenutno je brute force
    for (int j = area.HEIGHT - 1; j >= 0; j--)
    {
        if (area.countPerRow[j] == area.WIDTH_X * area.WIDTH_Z)
        {
            for (int jj = j; jj < area.HEIGHT - 2; jj++)
            {
                area.countPerRow[jj] = area.countPerRow[jj + 1];

                for (int i = 0; i < area.WIDTH_X; i++)
                    for (int k = 0; k < area.WIDTH_Z; k++)
                        area.positions[i][jj][k] = area.positions[i][jj + 1][k];
            }

            // clear top row
            for (int i = 0; i < area.WIDTH_X; i++)
                for (int k = 0; k < area.WIDTH_Z; k++)
                    area.positions[i][area.HEIGHT - 1][k] = 0;
        }
    }


    // Signal new level
    tickOffset = glfwGetTime();
    shouldSpawnNewBlock = true;
}

void Game::render(Shader &shader)
{
    shader.use();

    area.renderBorder(shader);

    if (!collisionDetected)
    {
        player.render(shader);
    }
    if (state != OVER)
        renderRotationAxis(shader);

    area.renderStaticBlocks(shader); // Must be called after rendering Player block to prevent visual stutter
}

void Game::transform(Transformation transform)
{
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


    if (transform == TRANS_FORWARD)
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
    else if (transform == TRANS_BACKWARD)
    {
        keyPressed = true;

        // Check for collision with border...
        for (int i = 0; i < SHAPE_WIDTH; i++)
            for (int j = 0; j < SHAPE_WIDTH; j++)
                for (int k = 0; k < SHAPE_WIDTH; k++)
                    if (player.shape.positions[i][j][k] && player.offset.z + k + 1 >= area.WIDTH_Z)
                        return;

        // ... or with other static blocks
        for (int i = 0; i < SHAPE_WIDTH; i++)
            for (int j = 0; j < SHAPE_WIDTH; j++)
                for (int k = 0; k < SHAPE_WIDTH; k++)
                    if (poy + j < area.HEIGHT && player.shape.positions[i][j][k] && area.positions[pox + i][poy + j][poz + k + 1])
                        return;
        
        player.offset.z += 1; // backward is in the direction of +z
    }
    else if (transform == TRANS_LEFT)
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
    else if (transform == TRANS_RIGHT)
    {
        keyPressed = true;

        // Check for collision with border...
        for (int i = 0; i < SHAPE_WIDTH; i++)
            for (int j = 0; j < SHAPE_WIDTH; j++)
                for (int k = 0; k < SHAPE_WIDTH; k++)
                    if (player.shape.positions[i][j][k] && player.offset.x + i + 1 >= area.WIDTH_X)
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
        std::cout << "rot ccw" << std::endl;
        keyPressed = true;
        //player.shape.rotate(player.rotationAxis, ROT_CCW);
        Shape rotated = player.shape.getRotated(player.rotationAxis, ROT_CCW);
        if (!detectHorizontalCollision(rotated))
            player.shape = rotated;
    }
    else if (transform == ROT_CW)
    {
        std::cout << "rot cw" << std::endl;
        keyPressed = true;
        //player.shape.rotate(player.rotationAxis, ROT_CW);
        Shape rotated = player.shape.getRotated(player.rotationAxis, ROT_CW);
        if (!detectHorizontalCollision(rotated))
            player.shape = rotated;
    }
}

void Game::drop()
{
    //if (!dropEnabled)
    //    return;
    
    //dropEnabled = false;

    int &pox = player.offset.x;
    int poy = player.offset.y; 
    int &poz = player.offset.z;

    int li = player.shape.getLowestIndex();

    for (int aj = poy; aj >= -SHAPE_WIDTH + 1; aj--)
    {
        bool found = false;

        int newOffset = 0;
            for (int i = 0; i < SHAPE_WIDTH && !found; i++)
                for (int k = 0; k < SHAPE_WIDTH && !found; k++)
                {
                    if (player.shape.positions[i][li][k] && (aj + li < 0 || area.positions[pox + i][aj + li][poz + k]))
                    {
                        found = true;
                        newOffset = aj;
                    }
                }
        
        if (found)
        {
            //poy = newOffset;
            dropOffset = poy - newOffset;
            std::cout << dropOffset << "\t" << poy << std::endl;
            break;
        }
    }
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
                        (pox + i < 0 || pox + i >= area.WIDTH_X || // pox + i + 1 >= area.WIDTH_X
                         poz + k < 0 || poz + k >= area.WIDTH_Z))
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
        model = glm::scale(model, glm::vec3(area.WIDTH_X));
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
        model = glm::scale(model, glm::vec3(area.WIDTH_Z));
    }
    
    glBindVertexArray(currAxisVAO);
    shader.setMat4("model", model);
    glDrawArrays(GL_LINES, 0, 2);
}