#version 330 core

struct Material {
    sampler2D diffuse;    // Texture for diffuse coloring
    sampler2D grass;      // Texture for grass
    float shininess;      // Shininess for specular reflection
};

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;        // Fragment position in world space
in vec3 Normal;         // Fragment normal in world space
in vec2 TexCoords;      // Texture coordinates
in float Height;        // Height value from vertex shader

out vec4 FragColor;     // Final fragment color

uniform vec3 viewPos;           // Position of the camera/viewer
uniform Material material;      // Material properties
uniform DirectionalLight dirLight; // Directional light properties

void main()
{
    // Ambient component
    vec3 ambient = dirLight.ambient * texture(material.diffuse, TexCoords).rgb;

    // Diffuse component
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-dirLight.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = dirLight.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

    // Specular component
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = dirLight.specular * spec;

    // Adjusted height-based grass blending
    float minHeight = 0.0;   // Minimum height for grass
    float maxHeight = 20.0;  // Maximum height for grass (matches terrain height)
    float heightFactor = clamp((Height - minHeight) / (maxHeight - minHeight), 0.0, 1.0);
    heightFactor = 1.0 - heightFactor; // Invert to have grass at lower heights

    vec3 baseColor = ambient + diffuse + specular; // Base lighting result
    vec3 grassColor = texture(material.grass, TexCoords).rgb; // Grass texture color

    // Blend the base color with the grass texture based on height
    vec3 finalColor = mix(baseColor, grassColor, heightFactor);

    FragColor = vec4(finalColor, 1.0); // Output the final fragment color
}
