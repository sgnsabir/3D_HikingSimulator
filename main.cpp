#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

// Include headers
#include "Camera.h"
#include "Shader.h"
#include "Terrain.h"
#include "Path.h"
#include "SkyDome.h"
#include "Light.h"
#include "PathTracer.h"

// Constants
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera
Camera camera(glm::vec3(0.0f, 20.0f, 0.0f));

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Mouse input
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Function declarations
void processInput(GLFWwindow* window, float deltaTime);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

int main()
{
    // GLFW initialization and configuration
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // For MacOS uncomment the following line
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // GLFW window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Hiking Simulator", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Capture the mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);

    // Build and compile shaders
    Shader terrainShader("shaders/terrain_vertex.glsl", "shaders/terrain_fragment.glsl");
    Shader pathShader("shaders/path_vertex.glsl", "shaders/path_fragment.glsl");
    Shader tracerShader("shaders/tracer_vertex.glsl", "shaders/tracer_fragment.glsl");
    Shader skyDomeShader("shaders/skydome_vertex.glsl", "shaders/skydome_fragment.glsl");

    // Load terrain
    Terrain terrain("assets/heightmaps/terrain_heightmap.png", "assets/textures/terrain_texture.png");
    terrain.LoadGrassTexture("assets/textures/grass_texture.png");

    // Load path
    Path path("assets/gpx/hiking_path.gpx", terrain);

    // Set camera position to the starting point of the hiking path
    glm::vec3 pathStartPosition = path.GetStartingPosition();
    float cameraHeightOffset = 2.0f; 
    camera.Position = glm::vec3(
        pathStartPosition.x,
        pathStartPosition.y + cameraHeightOffset,
        pathStartPosition.z
    );

    // Initialize SkyDome
    SkyDome skyDome("assets/skydome/sky_dome_texture.png");

    // Lighting
    DirectionalLight dirLight;
    dirLight.direction = glm::vec3(-0.7f, -1.0f, -0.7f);
    dirLight.ambient = glm::vec3(0.4f);
    dirLight.diffuse = glm::vec3(0.8f);
    dirLight.specular = glm::vec3(1.0f);

    // Path tracer to keep track of the camera's path
    PathTracer pathTracer;
    // Add initial position
    pathTracer.AddPoint(camera.Position);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window, deltaTime);

        // Keep the camera within the terrain bounds
        float maxX = static_cast<float>(terrain.GetWidth() - 1);
        float maxZ = static_cast<float>(terrain.GetHeight() - 1);

        if (camera.Position.x < 0.0f)
            camera.Position.x = 0.0f;
        if (camera.Position.x > maxX)
            camera.Position.x = maxX;
        if (camera.Position.z < 0.0f)
            camera.Position.z = 0.0f;
        if (camera.Position.z > maxZ)
            camera.Position.z = maxZ;

        // Update camera's Y-position based on terrain height
        float terrainHeight = terrain.GetHeightAt(camera.Position.x, camera.Position.z);
        float cameraHeightOffset = 2.0f; // Adjust as needed
        camera.Position.y = terrainHeight + cameraHeightOffset;

        // Record the camera position for the path tracer if the camera has moved significantly
        static glm::vec3 lastRecordedPosition = camera.Position;
        float distanceMoved = glm::length(camera.Position - lastRecordedPosition);
        if (distanceMoved > 0.5f) // Adjust threshold as needed
        {
            pathTracer.AddPoint(camera.Position);
            lastRecordedPosition = camera.Position;
        }

        // Clear buffers
        glClearColor(0.1f, 0.7f, 0.9f, 1.0f); // water color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // View and projection matrices
        glm::mat4 view = camera.GetViewMatrix();
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            static_cast<float>(width) / static_cast<float>(height), 0.1f, 1000.0f);

        // Render terrain
        terrainShader.Use();
        terrainShader.setMat4("projection", projection);
        terrainShader.setMat4("view", view);
        glm::mat4 model = glm::mat4(1.0f);
        terrainShader.setMat4("model", model);

        // Set directional light uniforms
        terrainShader.setVec3("dirLight.direction", dirLight.direction);
        terrainShader.setVec3("dirLight.ambient", dirLight.ambient);
        terrainShader.setVec3("dirLight.diffuse", dirLight.diffuse);
        terrainShader.setVec3("dirLight.specular", dirLight.specular);

        // Set view position
        terrainShader.setVec3("viewPos", camera.Position);

        // Set material properties
        terrainShader.setInt("material.diffuse", 0);
        terrainShader.setInt("material.grass", 1);
        terrainShader.setFloat("material.shininess", 32.0f);

        // Render terrain
        terrain.Render(terrainShader);

        // Render path from GPX
        pathShader.Use();
        pathShader.setMat4("projection", projection);
        pathShader.setMat4("view", view);
        pathShader.setMat4("model", model);
        pathShader.setVec3("color", glm::vec3(0.0f, 1.0f, 1.0f));
        path.Render(pathShader);

        // Render path tracer
        tracerShader.Use();
        tracerShader.setMat4("projection", projection);
        tracerShader.setMat4("view", view);
        tracerShader.setMat4("model", model);
        tracerShader.setVec3("color", glm::vec3(1.0f, 0.0f, 1.0f));
        glLineWidth(5.0f);
        pathTracer.Render(tracerShader);
        glLineWidth(1.0f);

        // Render sky dome
        glDepthMask(GL_FALSE); // Disable depth writing
        glm::mat4 skyView = glm::mat4(glm::mat3(view)); // Remove translation
        skyDomeShader.Use();
        skyDome.Render(skyDomeShader, skyView, projection);
        glDepthMask(GL_TRUE);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Terminate GLFW
    glfwTerminate();
    return 0;
}

// Process input
void processInput(GLFWwindow* window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Mouse callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // Reversed since y-coordinates go from bottom to top

    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// Framebuffer size callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
