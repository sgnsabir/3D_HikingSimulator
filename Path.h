#ifndef PATH_H
#define PATH_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Terrain.h"

class Path {
public:
    Path(const std::string& gpxPath, const Terrain& terrain);
    ~Path();
    void Render(Shader& shader);
    glm::vec3 GetStartingPosition() const;

private:
    GLuint VAO, VBO;
    std::vector<glm::vec3> pathPoints;

    bool loadGPX(const std::string& path);
    void setupPath();
    void adjustPointsToTerrain(const Terrain& terrain);
};

#endif 
