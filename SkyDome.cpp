#include "SkyDome.h"
#include <stb/stb_image.h>
#include <vector>
#include <iostream>
#include <glm/gtc/constants.hpp>

SkyDome::SkyDome(const std::string& texturePath) {
    generateSphereMesh(50, 50);
    loadTexture(texturePath);
}

SkyDome::~SkyDome() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void SkyDome::generateSphereMesh(unsigned int latitudeBands, unsigned int longitudeBands) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float radius = 250.0f;

    for (unsigned int latNumber = 0; latNumber <= latitudeBands/2; ++latNumber) {
        float theta = latNumber * glm::pi<float>() / latitudeBands;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (unsigned int longNumber = 0; longNumber <= longitudeBands; ++longNumber) {
            float phi = longNumber * 2 * glm::pi<float>() / longitudeBands;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;

            float u = 1.0f - ((float)longNumber / longitudeBands);
            float v = 1.0f - ((float)latNumber / latitudeBands);
            //float v = 2.0f * (1.0f - ((float)latNumber / (latitudeBands / 2)));


            vertices.push_back(radius * x);
            vertices.push_back(radius * y);
            vertices.push_back(radius * z);
            vertices.push_back(u);
            vertices.push_back(v);
        }
    }

    for (unsigned int latNumber = 0; latNumber < latitudeBands; ++latNumber) {
        for (unsigned int longNumber = 0; longNumber < longitudeBands; ++longNumber) {
            unsigned int first = (latNumber * (longitudeBands + 1)) + longNumber;
            unsigned int second = first + longitudeBands + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    indexCount = static_cast<unsigned int>(indices.size());

    // Create buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Vertex attributes
    // Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0));

    // Texture Coordinates
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

void SkyDome::loadTexture(const std::string& path) {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrap around
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Clamp to prevent seams
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(false); // Flip texture if necessary

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cerr << "Sky dome texture failed to load at path: " << path << std::endl;
    }
    stbi_image_free(data);
}

void SkyDome::Render(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    glDepthFunc(GL_LEQUAL);   // Ensure sky dome is rendered behind all other objects
    glDisable(GL_CULL_FACE);  // Disable face culling

    shader.Use();

    // Remove translation from the view matrix
    glm::mat4 rotView = glm::mat4(glm::mat3(view));
    shader.setMat4("view", rotView);
    shader.setMat4("projection", projection);

    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    shader.setInt("skyTexture", 0); // Ensure the sampler2D uniform is set
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glEnable(GL_CULL_FACE);   // Re-enable face culling
    glDepthFunc(GL_LESS);     // Restore default depth function
}
