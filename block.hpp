#ifndef BLOCK_H
#define BLOCK_H

#include <glad/glad.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

#include <glm/glm.hpp>

unsigned int loadTexture(const char *path);

std::string getTagName(const std::string &in);
std::string getTagValue(const std::string &in);
std::vector<std::string> split(const std::string &str, const std::string &delimiter);


struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 textureCoords;
};
struct Material
{
    std::string name;
    GLuint diffuseTextureID;
    GLuint specularTextureID;
    float Ns; // Specular exponent
};

std::vector<Material> materials;

class Block
{
    private:

    public:
        GLuint VBO;
        GLuint VAO;
        std::vector<Vertex> vertices;
        Material material;

        Block() {};
        Block(const std::string &objPath, GLuint defaultTextureID)
        {
            loadObj(objPath);
            loadMtl("resources/objects/block/materials.mtl");

            std::cout << "Materials (" << materials.size() << "):" << std::endl;
            for (Material m : materials)
            {
                std::cout << m.name << std::endl;
            }
            //textureID = defaultTextureID;

            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
            glBindVertexArray(VAO);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, textureCoords));
            glEnableVertexAttribArray(2);

            glBindVertexArray(0);
        }

        void draw()
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, material.diffuseTextureID);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, material.specularTextureID);

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());
            glActiveTexture(GL_TEXTURE0);
        }
        void del()
        {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
        }

        bool loadObj(const std::string &objPath);
        bool loadMtl(const std::string &mtlPath);
};

// Reduced version of loadObj from Model (only loads vertex data)
bool Block::loadObj(const std::string &objPath)
{
    // Check if file is of valid type
    if (objPath.substr(objPath.length() - 4, 4) != ".obj")
        return false;

    std::ifstream file(objPath);

    if (!file)
        return false;
    
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;

    //std::vector<Vertex> vertices; // temp array for vertices which are to be stored in a Mesh (faces which use the same texture)

    std::string curLine;
    while (std::getline(file, curLine))
    {
        std::string tagName = getTagName(curLine);
        if (tagName.length() == 0 || tagName[0] == '#')
            continue;

        std::string tagValue = getTagValue(curLine);

        // Get vertex position (v)
        if (tagName == "v")
        {
            std::vector<std::string> sPos = split(tagValue, " \t");
            if (sPos.size() != 3)
            {
                std::cout << "Invalid Vertex position data" << std::endl;
                continue;
            }
            
            glm::vec3 vPos;
            vPos.x = std::stof(sPos[0]);
            vPos.y = std::stof(sPos[1]);
            vPos.z = std::stof(sPos[2]);
            positions.push_back(vPos);
        }

        // Get vertex texture coordinates (vt)
        if (tagName == "vt")
        {
            std::vector<std::string> sTexCoords = split(tagValue, " \t");
            if (sTexCoords.size() != 2)
            {
                std::cout << "Invalid Vertex texture coordinates data" << std::endl;
                continue;
            }
            
            glm::vec2 vTexCoords;
            vTexCoords.x = std::stof(sTexCoords[0]);
            vTexCoords.y = std::stof(sTexCoords[1]);
            texCoords.push_back(vTexCoords);
        }

        // Get vertex normal
        if (tagName == "vn")
        {
            std::vector<std::string> sNorm = split(tagValue, " \t");
            if (sNorm.size() != 3)
            {
                std::cout << "Invalid Vertex position data" << std::endl;
                continue;
            }
            
            glm::vec3 vNorm;
            vNorm.x = std::stof(sNorm[0]);
            vNorm.y = std::stof(sNorm[1]);
            vNorm.z = std::stof(sNorm[2]);
            normals.push_back(vNorm);
        }

        // Face
        if (tagName == "f")
        {
            std::vector<std::string> sFaces = split(tagValue, " \t");
            if (sFaces.size() != 3)
            {
                std::cout << "Invalid Face data" << std::endl;
                continue;
            }

            for (std::string sFace : sFaces)
            {
                std::vector<std::string> sVert = split(sFace, "/");
                if (sVert.size() != 3)
                {
                    std::cout << "Invalid Vertex data" << std::endl;
                    continue;
                }

                std::string sPos       = sVert[0];
                std::string sNorm      = sVert[2];
                std::string sTexCoords = sVert[1];

                glm::vec3 pos = positions[std::stoi(sPos) - 1];
                glm::vec3 norm = normals[std::stoi(sNorm) - 1];
                glm::vec2 tex = texCoords[std::stoi(sTexCoords) - 1];

                Vertex v = { .position = pos,
                                .normal = norm,
                                .textureCoords = tex };

                vertices.push_back(v);
            }
        }
    }

    file.close();

    return true;
}

bool Block::loadMtl(const std::string &mtlPath)
{
    // Check if file is of valid type
    if (mtlPath.substr(mtlPath.length() - 4, 4) != ".mtl")
        return false;

    std::ifstream file(mtlPath);

    if (!file)
        return false;

    Material currMat;

    std::string currLine;
    while (std::getline(file, currLine))
    {
        std::string tagName = getTagName(currLine);
        if (tagName.length() == 0 || tagName[0] == '#')
            continue;

        std::string tagValue = getTagValue(currLine);

        // Instantiate new material
        if (tagName == "newmtl")
        {
            if (currMat.name.length() > 0)
            {
                materials.push_back(currMat);

                currMat = Material();
            }

            if (tagValue.length() > 0)
                currMat.name = tagValue;
            else
                currMat.name = "unnamed";
        }

        // Specular exponent
        if (tagName == "Ns")
        {
            currMat.Ns = std::stof(tagValue);
        }

        // Diffuse texture
        if (tagName == "map_Kd")
        {
            std::cout << "diff\t" << tagValue << std::endl;
            currMat.diffuseTextureID = loadTexture(tagValue.c_str());
        }
        
        // Specular texture
        if (tagName == "map_Ks")
        {
            std::cout << "spec\t" << tagValue << std::endl;
            currMat.specularTextureID = loadTexture(tagValue.c_str());
        }
    }

    file.close();

    materials.push_back(currMat);
    
    return true;
}

std::string getTagName(const std::string &in)
{
    size_t nameStart = in.find_first_not_of(" \t");
    size_t nameEnd = in.find_first_of(" \t", nameStart);

    if (nameStart != std::string::npos && nameEnd != std::string::npos)
        return in.substr(nameStart, nameEnd - nameStart);
    else if (nameStart != std::string::npos)
        return in.substr(nameStart);
    
    return "";
}

std::string getTagValue(const std::string &in)
{
    size_t nameStart = in.find_first_not_of(" \t");
    size_t nameEnd = in.find_first_of(" \t", nameStart);

    size_t valStart = in.find_first_not_of(" \t", nameEnd);
    size_t valEnd = in.find_last_not_of(" \t") + 1;

    if (valStart != std::string::npos && valEnd != std::string::npos)
        return in.substr(valStart, valEnd - valStart);
    else if (valStart != std::string::npos)
        return in.substr(valStart);
    
    return "";
}

std::vector<std::string> split(const std::string &str, const std::string &delimiter)
{
    std::vector<std::string> values;

    size_t start = str.find_first_not_of(delimiter);
    size_t end = str.find_first_of(delimiter);

    while (start != end)
    {
        values.push_back(str.substr(start, end - start));

        start = str.find_first_not_of(delimiter, end);
        end = str.find_first_of(delimiter, start);
    }

    return values;
}

#endif