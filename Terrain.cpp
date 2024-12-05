#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include "Terrain.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

// Constructor
Terrain::Terrain(const std::string& heightmapPath, const std::string& texturePath) {
    stbi_set_flip_vertically_on_load(true);
    if (!loadHeightmap(heightmapPath)) {
        std::cerr << "ERROR::TERRAIN::FAILED_TO_LOAD_HEIGHTMAP: " << heightmapPath << std::endl;
    }
    else {
        std::cout << "Loaded heightmap from: " << heightmapPath << " [Width: " << width << ", Height: " << height << "]" << std::endl;
    }

    if (!loadTexture(texturePath)) {
        std::cerr << "ERROR::TERRAIN::FAILED_TO_LOAD_TEXTURE: " << texturePath << std::endl;
    }
    else {
        std::cout << "Loaded texture from: " << texturePath << std::endl;
    }

    setupMesh();
}

// Destructor
Terrain::~Terrain() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &texture);
    glDeleteTextures(1, &grassTexture);
}

// Load grass texture
void Terrain::LoadGrassTexture(const std::string& grassTexturePath) {
    glGenTextures(1, &grassTexture);
    glBindTexture(GL_TEXTURE_2D, grassTexture);

    int texWidth, texHeight, channels;
    unsigned char* image = stbi_load(grassTexturePath.c_str(), &texWidth, &texHeight, &channels, 0);
    if (!image) {
        std::cerr << "ERROR::TERRAIN::FAILED_TO_LOAD_GRASS_TEXTURE: " << grassTexturePath << std::endl;
        return;
    }

    GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    std::cout << "Loaded grass texture from: " << grassTexturePath << std::endl;
}

// Render function
void Terrain::Render(Shader& shader) {
    shader.Use();

    // Bind the default terrain texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shader.setInt("material.diffuse", 0);

    // Bind the grass texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    shader.setInt("material.grass", 1);

    shader.setFloat("material.shininess", 32.0f);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

// Load heightmap
bool Terrain::loadHeightmap(const std::string& path) {
    int channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_grey);
    if (!data) {
        std::cerr << "ERROR::TERRAIN::FAILED_TO_LOAD_HEIGHTMAP: " << path << std::endl;
        return false;
    }

    heightData.resize(width * height);
    for (int i = 0; i < width * height; ++i) {
        heightData[i] = static_cast<float>(data[i]) / 255.0f * 20.0f; // Adjusted scaling to match shader
    }
    stbi_image_free(data);
    return true;
}

// Load texture
bool Terrain::loadTexture(const std::string& texturePath) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    int texWidth, texHeight, channels;
    unsigned char* image = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &channels, 0);
    if (!image) {
        std::cerr << "ERROR::TERRAIN::FAILED_TO_LOAD_TEXTURE: " << texturePath << std::endl;
        return false;
    }

    GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

// Setup mesh
void Terrain::setupMesh() {
    vertices.clear();
    indices.clear();
    float uvScale = -2.0f;
    // Generate vertices with positions, UVs, and placeholder normals
    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            float y = getHeight(x, z);
            glm::vec3 position = glm::vec3(static_cast<float>(x), y, static_cast<float>(z));
            //glm::vec2 texCoords = glm::vec2(static_cast<float>(x) / (static_cast<float>(width) - 1.0f), static_cast<float>(z) / (static_cast<float>(height) - 1.0f));
            glm::vec2 texCoords = glm::vec2(
                (static_cast<float>(x) / (static_cast<float>(width) - 1.0f)) * uvScale,
                (static_cast<float>(z) / (static_cast<float>(height) - 1.0f)) * uvScale
            ); 
            Vertex vertex;
            vertex.Position = position;
            vertex.TexCoords = texCoords;
            vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f); // Initial normal pointing up
            vertices.push_back(vertex);
        }
    }

    // Generate indices for two triangles per quad with corrected winding order
    for (int z = 0; z < height - 1; ++z) {
        for (int x = 0; x < width - 1; ++x) {
            int topLeft = z * width + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * width + x;
            int bottomRight = bottomLeft + 1;

            // First triangle
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Second triangle
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    // Compute normals for lighting
    computeNormals();

    // Generate and bind buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Set vertex attribute pointers
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);
    // TexCoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

// Compute normals by averaging the normals of adjacent triangles
void Terrain::computeNormals() {
    // Initialize all normals to zero
    for (auto& vertex : vertices) {
        vertex.Normal = glm::vec3(0.0f);
    }

    // Iterate over each triangle and accumulate normals
    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int idx0 = indices[i];
        unsigned int idx1 = indices[i + 1];
        unsigned int idx2 = indices[i + 2];

        glm::vec3 v0 = vertices[idx0].Position;
        glm::vec3 v1 = vertices[idx1].Position;
        glm::vec3 v2 = vertices[idx2].Position;

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        vertices[idx0].Normal += normal;
        vertices[idx1].Normal += normal;
        vertices[idx2].Normal += normal;
    }

    // Normalize all normals
    for (auto& vertex : vertices) {
        vertex.Normal = glm::normalize(vertex.Normal);
    }
}

// Get height at specific coordinates
float Terrain::getHeight(int x, int z) const {
    if (x < 0 || x >= width || z < 0 || z >= height) {
        return 0.0f;
    }
    return heightData[z * width + x];
}

float Terrain::GetHeightAt(float x, float z) const {
    int ix = static_cast<int>(x);
    int iz = static_cast<int>(z);
    return getHeight(ix, iz);
}
