#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

#include <glm/glm.hpp>

unsigned int loadTexture(const char *path);

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 textureCoords;
};

struct Material
{
    std::string name;
    glm::vec3 Ka; // Ambient color
    glm::vec3 Kd; // Diffuse color
    glm::vec3 Ks; // Specular color
    float Ns; // Specular exponent
    std::string map_Kd; // Diffuse texture map path

    GLuint diffuseTextureID;

    Material() {};

    void print()
    {
        std::cout << "Material \"" << name << "\": " << std::endl
            << "\tKa\t" << Ka.x << " " << Ka.y << " " << Ka.z << std::endl
            << "\tKd\t" << Kd.x << " " << Kd.y << " " << Kd.z << std::endl
            << "\tKs\t" << Ks.x << " " << Ks.y << " " << Ks.z << std::endl
            << "\tNs\t" << Ns << std::endl
            << "\tmap_Kd\t" << map_Kd << std::endl;
    }
};

class Mesh
{
    private:
        GLuint VBO;
        GLuint VAO;
        GLuint textureID;

    public:
        std::string name;
        std::vector<Vertex> vertices;
        Material material;

        Mesh() {};
        Mesh(std::vector<Vertex> &v)
        {
            vertices = v;

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
            glBindVertexArray(VAO);
            glBindTexture(GL_TEXTURE_2D, material.diffuseTextureID);
            glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        }
        void del()
        {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
        }
};


std::string getTagName(const std::string &in);
std::string getTagValue(const std::string &in);
std::vector<std::string> split(const std::string &str, const std::string &delimiter);


class Model
{
    public:
        std::vector<Vertex> LoadedVertices;   
        std::vector<Material> LoadedMaterials;
        std::vector<Mesh> LoadedMeshes;

        Model(const std::string &objPath)
        {
            loadObj(objPath);
        }
        ~Model()
        {

        }

        void draw()
        {
            for (Mesh mesh : this->LoadedMeshes)
            {
                mesh.draw();
            }
        }
        void del()
        {
            for (Mesh mesh : this->LoadedMeshes)
            {
                mesh.del();
            }
        }

    private:
        bool loadObj(const std::string &objPath);
        bool loadMtl(const std::string &mtlPath);
};

bool Model::loadObj(const std::string &objPath)
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

    std::vector<Vertex> vertices; // temp array for vertices which are to be stored in a Mesh (faces which use the same texture)

    std::vector<std::string> meshMatNames;
    std::string meshName;
    Mesh currMesh;

    std::string curLine;
    while (std::getline(file, curLine))
    {
        std::string tagName = getTagName(curLine);
        if (tagName.length() == 0 || tagName[0] == '#')
            continue;

        std::string tagValue = getTagValue(curLine);
        

        // Generate mesh
        if (tagName == "o")
        {
            if (!tagValue.empty())
                meshName = tagValue;
            else
                meshName = "unnamed";
        }

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
                LoadedVertices.push_back(v);
            }

            //std::vector<std::string> sVertPos = split(sFaces[0], "/");
            
            glm::vec3 vNorm;
            vNorm.x = std::stof(sFaces[0]);
            vNorm.y = std::stof(sFaces[1]);
            vNorm.z = std::stof(sFaces[2]);
            normals.push_back(vNorm);
        }

        if (tagName == "mtllib")
        {
            size_t objectNameStart = objPath.find_last_of("/") + 1;
            unsigned int objectNameLength = objPath.length() - objectNameStart - 4;

            std::string folderPath = objPath.substr(0, objectNameStart - 1);
            std::string objectName = objPath.substr(objectNameStart, objectNameLength);
            std::string mtlPath = folderPath + "/" + objectName + ".mtl";

            std::cout << "Folder: " << folderPath << std::endl << "mtl: " << mtlPath << std::endl;
            
            if (!loadMtl(mtlPath))
            {
                std::cout << "Failed to load materials at " << mtlPath << std::endl;
            }
        }

        if (tagName == "usemtl")
        {
            meshMatNames.push_back(tagValue);

            // Material changed, create new Mesh
            if (!vertices.empty())
            {
                currMesh = Mesh(vertices);
                currMesh.name = meshName;

                LoadedMeshes.push_back(currMesh);

                vertices.clear();
            }
        }
    }

    file.close();

    // Deal with last Mesh
    if (!vertices.empty())
    {
        currMesh = Mesh(vertices);
        currMesh.name = meshName;

        LoadedMeshes.push_back(currMesh);
    }

    // Set Material for each Mesh
    // meshMatNames[x] == LoadedMeshes[x].material.name
    for (int i = 0; i < meshMatNames.size(); i++)
    {
        std::string meshMatName = meshMatNames[i];

        for (int j = 0; j < LoadedMaterials.size(); j++)
        {
            if (LoadedMaterials[j].name == meshMatName)
            {
                LoadedMaterials[j].diffuseTextureID = loadTexture(LoadedMaterials[j].map_Kd.c_str());
                LoadedMeshes[i].material = LoadedMaterials[j];
            }
        }
    }

    return true;
}

bool Model::loadMtl(const std::string &mtlPath)
{
    // Check if file is of valid type
    if (mtlPath.substr(mtlPath.length() - 4, 4) != ".mtl")
        return false;

    std::ifstream file(mtlPath);

    if (!file)
        return false;

    std::vector<Material> materials;
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

        // Ambient color
        if (tagName == "Ka")
        {
            std::vector<std::string> sKa = split(tagValue, " \t");
            if (sKa.size() != 3)
            {
                std::cout << "Invalid Material ambient color data" << std::endl;
                continue;
            }

            currMat.Ka.x = std::stof(sKa[0]);
            currMat.Ka.y = std::stof(sKa[1]);
            currMat.Ka.z = std::stof(sKa[2]);
        }

        // Diffuse color
        if (tagName == "Kd")
        {
            std::vector<std::string> sKd = split(tagValue, " \t");
            if (sKd.size() != 3)
            {
                std::cout << "Invalid Material diffuse color data" << std::endl;
                continue;
            }

            currMat.Kd.x = std::stof(sKd[0]);
            currMat.Kd.y = std::stof(sKd[1]);
            currMat.Kd.z = std::stof(sKd[2]);
        }

        // Specular color
        if (tagName == "Ks")
        {
            std::vector<std::string> sKs = split(tagValue, " \t");
            if (sKs.size() != 3)
            {
                std::cout << "Invalid Material specular color data" << std::endl;
                continue;
            }

            currMat.Ks.x = std::stof(sKs[0]);
            currMat.Ks.y = std::stof(sKs[1]);
            currMat.Ks.z = std::stof(sKs[2]);
        }

        // Specular exponent
        if (tagName == "Ns")
        {
            currMat.Ns = std::stof(tagValue);
        }

        // Specular exponent
        if (tagName == "map_Kd")
        {
            currMat.map_Kd = tagValue;
            currMat.Kd = glm::vec3(0.0f);
        }
        
    }

    file.close();

    materials.push_back(currMat);

    LoadedMaterials = materials;
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