#ifndef SKYDOME_H
#define SKYDOME_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include "Shader.h"

class SkyDome {
public:
    SkyDome(const std::string& texturePath);
    ~SkyDome();
    void Render(Shader& shader, const glm::mat4& view, const glm::mat4& projection);

private:
    GLuint VAO, VBO, EBO;
    GLuint textureID;
    unsigned int indexCount;
    void loadTexture(const std::string& path);
    void generateSphereMesh(unsigned int latitudeBands, unsigned int longitudeBands);
};

#endif 
