#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <string>
#include "Shader.h"

class Light {
public:
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    Light(const glm::vec3& ambient = glm::vec3(0.05f),
        const glm::vec3& diffuse = glm::vec3(0.8f),
        const glm::vec3& specular = glm::vec3(1.0f))
        : ambient(ambient), diffuse(diffuse), specular(specular) {
    }

    virtual void ApplyToShader(Shader& shader, const std::string& name) = 0;
};

class DirectionalLight : public Light {
public:
    glm::vec3 direction;

    DirectionalLight(const glm::vec3& direction = glm::vec3(-0.2f, -1.0f, -0.3f),
        const glm::vec3& ambient = glm::vec3(0.05f),
        const glm::vec3& diffuse = glm::vec3(0.4f),
        const glm::vec3& specular = glm::vec3(0.5f))
        : Light(ambient, diffuse, specular), direction(direction) {
    }

    void ApplyToShader(Shader& shader, const std::string& name) override;
};

class PointLight : public Light {
public:
    glm::vec3 position;

    float constant;
    float linear;
    float quadratic;

    PointLight(const glm::vec3& position = glm::vec3(0.0f),
        const glm::vec3& ambient = glm::vec3(0.05f),
        const glm::vec3& diffuse = glm::vec3(0.8f),
        const glm::vec3& specular = glm::vec3(1.0f),
        float constant = 1.0f,
        float linear = 0.09f,
        float quadratic = 0.032f)
        : Light(ambient, diffuse, specular), position(position),
        constant(constant), linear(linear), quadratic(quadratic) {
    }

    void ApplyToShader(Shader& shader, const std::string& name) override;
};

class SpotLight : public Light {
public:
    glm::vec3 position;
    glm::vec3 direction;

    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    SpotLight(const glm::vec3& position = glm::vec3(0.0f),
        const glm::vec3& direction = glm::vec3(0.0f, 0.0f, -1.0f),
        const glm::vec3& ambient = glm::vec3(0.0f),
        const glm::vec3& diffuse = glm::vec3(1.0f),
        const glm::vec3& specular = glm::vec3(1.0f),
        float cutOff = glm::cos(glm::radians(12.5f)),
        float outerCutOff = glm::cos(glm::radians(17.5f)),
        float constant = 1.0f,
        float linear = 0.09f,
        float quadratic = 0.032f)
        : Light(ambient, diffuse, specular), position(position), direction(direction),
        cutOff(cutOff), outerCutOff(outerCutOff),
        constant(constant), linear(linear), quadratic(quadratic) {
    }

    void ApplyToShader(Shader& shader, const std::string& name) override;
};

#endif 