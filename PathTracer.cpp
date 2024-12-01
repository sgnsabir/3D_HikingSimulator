#include "PathTracer.h"

PathTracer::PathTracer() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
}

void PathTracer::AddPoint(const glm::vec3& point) {
    pathPoints.push_back(point);
    updateBuffer();
}

void PathTracer::updateBuffer() {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, pathPoints.size() * sizeof(glm::vec3), pathPoints.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glBindVertexArray(0);
}

void PathTracer::Render(Shader& shader) {
    shader.Use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(pathPoints.size()));
    glBindVertexArray(0);
}
