#include "Light.h"

void DirectionalLight::ApplyToShader(Shader& shader, const std::string& name) {
    shader.setVec3(name + ".direction", direction);
    shader.setVec3(name + ".ambient", ambient);
    shader.setVec3(name + ".diffuse", diffuse);
    shader.setVec3(name + ".specular", specular);
}

void PointLight::ApplyToShader(Shader& shader, const std::string& name) {
    shader.setVec3(name + ".position", position);
    shader.setVec3(name + ".ambient", ambient);
    shader.setVec3(name + ".diffuse", diffuse);
    shader.setVec3(name + ".specular", specular);
    shader.setFloat(name + ".constant", constant);
    shader.setFloat(name + ".linear", linear);
    shader.setFloat(name + ".quadratic", quadratic);
}

void SpotLight::ApplyToShader(Shader& shader, const std::string& name) {
    shader.setVec3(name + ".position", position);
    shader.setVec3(name + ".direction", direction);
    shader.setVec3(name + ".ambient", ambient);
    shader.setVec3(name + ".diffuse", diffuse);
    shader.setVec3(name + ".specular", specular);
    shader.setFloat(name + ".cutOff", cutOff);
    shader.setFloat(name + ".outerCutOff", outerCutOff);
    shader.setFloat(name + ".constant", constant);
    shader.setFloat(name + ".linear", linear);
    shader.setFloat(name + ".quadratic", quadratic);
}
