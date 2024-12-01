#include "Path.h"
#include <iostream>
#include <pugixml/src/pugixml.hpp>
#include <limits>

Path::Path(const std::string& gpxPath, const Terrain& terrain) {
    if (!loadGPX(gpxPath)) {
        std::cerr << "Error loading GPX file. Path will not be rendered.\n";
        return;
    }
    adjustPointsToTerrain(terrain);
    setupPath();
}

Path::~Path() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

bool Path::loadGPX(const std::string& path) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(path.c_str());

    if (!result) {
        std::cerr << "Failed to load GPX file: " << path << "\n";
        return false;
    }

    // Use XPath to select all 'trkpt' elements, regardless of namespace
    pugi::xpath_node_set trkpts = doc.select_nodes("//*[local-name()='trkpt']");

    if (trkpts.empty()) {
        std::cerr << "No track points found in GPX file\n";
        return false;
    }

    for (pugi::xpath_node xpath_node : trkpts) {
        pugi::xml_node trkpt = xpath_node.node();

        double lat = trkpt.attribute("lat").as_double();
        double lon = trkpt.attribute("lon").as_double();
        double ele = 0.0;

        // Find 'ele' child regardless of namespace
        pugi::xpath_node ele_node = trkpt.select_node("*[local-name()='ele']");
        if (ele_node) {
            ele = ele_node.node().text().as_double();
        }

        pathPoints.emplace_back(static_cast<float>(lon), static_cast<float>(ele), static_cast<float>(lat));
    }

    return true;
}

void Path::adjustPointsToTerrain(const Terrain& terrain) {
    // Determine GPS bounds
    double minLat = std::numeric_limits<double>::max();
    double maxLat = std::numeric_limits<double>::lowest();
    double minLon = std::numeric_limits<double>::max();
    double maxLon = std::numeric_limits<double>::lowest();

    // First pass to find bounds
    for (const auto& point : pathPoints) {
        minLat = std::min(minLat, static_cast<double>(point.z));
        maxLat = std::max(maxLat, static_cast<double>(point.z));
        minLon = std::min(minLon, static_cast<double>(point.x));
        maxLon = std::max(maxLon, static_cast<double>(point.x));
    }

    float terrainWidth = static_cast<float>(terrain.GetWidth());
    float terrainHeight = static_cast<float>(terrain.GetHeight());

    // Calculate scaling factors
    double scaleX = terrainWidth / (maxLon - minLon);
    double scaleZ = terrainHeight / (maxLat - minLat);

    // Adjust points to terrain coordinates
    for (auto& point : pathPoints) {
        point.x = static_cast<float>((point.x - minLon) * scaleX);
        point.z = static_cast<float>((point.z - minLat) * scaleZ);

        // Adjust Y coordinate based on terrain height
        float terrainHeightAtPoint = terrain.GetHeightAt(point.x, point.z);
        point.y = terrainHeightAtPoint + 0.5f; // Offset above terrain
    }
}

void Path::setupPath() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, pathPoints.size() * sizeof(glm::vec3), pathPoints.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Path::Render(Shader& shader) {
    shader.Use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(pathPoints.size()));
    glBindVertexArray(0);
}

glm::vec3 Path::GetStartingPosition() const {
    if (!pathPoints.empty()) {
        return pathPoints.front();
    }
    else {
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
}
