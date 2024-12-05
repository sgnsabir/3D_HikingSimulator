#version 330 core

layout(location = 0) in vec3 aPos;       // Vertex position
layout(location = 1) in vec3 aNormal;    // Vertex normal
layout(location = 2) in vec2 aTexCoords; // Texture coordinates

out vec3 FragPos;        // Position of the fragment in world space
out vec3 Normal;         // Normal of the fragment in world space
out vec2 TexCoords;      // Texture coordinates
out float Height;        // Terrain height (y-coordinate) for height-based texturing

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0)); // Calculate world position of the vertex
    Normal = mat3(transpose(inverse(model))) * aNormal; // Transform normal to world space
    TexCoords = aTexCoords;
    // Match with height scaling in terrain generation
    float terrainScale = 20.0;
    Height = FragPos.y / terrainScale;

    gl_Position = projection * view * vec4(FragPos, 1.0); // Transform vertex to clip space
}
