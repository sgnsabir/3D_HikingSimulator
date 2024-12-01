#ifndef PATHTRACER_H
#define PATHTRACER_H

#include <vector>
#include <glm/glm.hpp>
#include "Shader.h"

class PathTracer {
public:
    PathTracer();
    void AddPoint(const glm::vec3& point);
    void Render(Shader& shader);

private:
    std::vector<glm::vec3> pathPoints;
    GLuint VAO, VBO;
    void updateBuffer();
};

#endif 
