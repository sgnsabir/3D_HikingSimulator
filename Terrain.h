#ifndef TERRAIN_H
#define TERRAIN_H

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "Shader.h"
#include <glad/glad.h>

// Define a Vertex structure
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class Terrain {
public:
    // Constructor and Destructor
    Terrain(const std::string& heightmapPath, const std::string& texturePath);
    ~Terrain();

    // Render function
    void Render(Shader& shader);

    // Load additional textures
    void LoadGrassTexture(const std::string& grassTexturePath);

    // Getter methods
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    float GetHeightAt(float x, float z) const;

    // Get grass texture ID
    GLuint GetGrassTexture() const { return grassTexture; }

private:
    // OpenGL objects
    GLuint VAO, VBO, EBO;
    GLuint texture;      // Base terrain texture
    GLuint grassTexture; // Grass texture

    // Heightmap and mesh data
    int width, height;
    std::vector<float> heightData;
    std::vector<unsigned int> indices;
    std::vector<Vertex> vertices;

    // Helper functions
    bool loadTexture(const std::string& texturePath);
    bool loadHeightmap(const std::string& path);
    void setupMesh();
    void computeNormals();
    float getHeight(int x, int z) const;
};

#endif
