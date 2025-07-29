/**
 * @file main.cpp
 * @brief Main entry point for the OpenGL Solar System simulation.
 *
 * This file handles window creation, OpenGL initialization, resource loading,
 * input processing, and the main render loop.
 *
 * @mainpage OpenGL Solar System
 *
 * This project is a real-time 3D simulation of our solar system,
 * built using C++ and OpenGL. It aims for a balance between visual
 * appeal and physical accuracy where appropriate.
 *
 */

// STL
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

// OpenGL Libraries
#include <GL/glew.h>
#ifdef __APPLE__
    #include <OpenGL/gl.h> // Just for macOS
#else
    #include <GL/gl.h>
#endif
#include <GLFW/glfw3.h>

// GLM for math
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Project headers
#include "Camera.hpp"
#include "Shader.hpp"
#include "Planet.hpp"
#include "Earth.hpp"
#include "Skybox.hpp"

// Image loading library
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 800;

// Constants
float ASTRONOMICAL_UNIT = 120.0f; // Astronomical Unit, used to scale the solar system

// Global states
// Camera, with a starting position looking down
Camera camera(glm::vec3(0.0f, 60.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -89.0f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// GLFW Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

// Helper Functions
void processInput(GLFWwindow *window);
GLenum glCheckError_(const char *file, int line);
GLFWwindow* initWindowManager();
void setUpGlowEffect(unsigned int& vao, unsigned int& textureID, Shader& shader);

// Macro for error checking
#define glCheckError() glCheckError_(__FILE__, __LINE__)

int main(void) {
    GLFWwindow* window = initWindowManager();

    if (!window) {
        return -1;
    }

    // Depth testing, face culling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCheckError();

    // Shaders
    Shader earthShader("../shaders/lighting_earth.vs", "../shaders/lighting_earth.fs");
    Shader planetShader("../shaders/lighting_planet.vs", "../shaders/lighting_planet.fs");
    Shader sunShader("../shaders/lighting_planet.vs", "../shaders/lighting_sun.fs");
    Shader skyboxShader("../shaders/skybox.vs", "../shaders/skybox.fs");
    Shader glowShader("../shaders/glow.vs", "../shaders/glow.fs");
    glCheckError();

    struct PlanetInfo {
        std::string modelPath;
        float scale;
        float orbitRadius;
        float orbitSpeed;
        float rotationSpeed;
        float axialTilt;
        bool hasGlow;
        float glowScale;
        glm::vec4 glowColor;
        float ellipticity;
    };

    std::vector<PlanetInfo> planetData = {
        {"../models/Mercury_1_4878.glb", 0.0038f, 0.39f, 42.0f, 10.0f, 0.03f, false, 0.0f, {}, 0.8f},
        {"../models/Venus_1_12103.glb", 0.0095f, 0.72f, 16.0f, 10.0f, 177.4f, false, 0.0f, {}, 0.95f},
        {"../models/24881_Mars_1_6792.glb", 0.0053f, 1.52f, 5.0f, 10.0f, 25.2f, true, 5.0f, glm::vec4(0.9f, 0.4f, 0.2f, 0.4f), 0.92f},
        {"../models/Jupiter_1_142984.glb", 0.112f, 5.20f, 1.0f, 10.0f, 3.1f, false, 0.0f, {}, 0.96f},
        {"../models/Saturn_1_120536.glb", 0.093f, 9.58f, 0.6f, 10.0f, 26.7f, false, 0.0f, {}, 0.95f},
        {"../models/Uranus_1_51118.glb", 0.04f, 19.2f, 0.2f, 10.0f, 97.8f, false, 0.0f, {}, 0.94f},
        {"../models/Neptune_1_49528.glb", 0.038f, 30.1f, 0.1f, 10.0f, 28.3f, false, 0.0f, {}, 0.96f}
    };

    std::vector<Planet> planets;
    for (const auto& data : planetData) {
        planets.emplace_back(data.modelPath, data.scale, ASTRONOMICAL_UNIT * data.orbitRadius, data.orbitSpeed, data.rotationSpeed,
                             data.axialTilt, data.hasGlow, data.glowScale, data.glowColor, data.ellipticity);
    }

    Planet sun ("../models/Sun_1_1391000.glb", 50.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    Earth earth("../models/earth(1).glb", "../images/2k_earth_daymap.jpg", "../images/2k_earth_nightmap.jpg",
                "../images/2k_earth_clouds.jpg", "../images/earthspec1k.jpg", 4.01f, ASTRONOMICAL_UNIT * 1.0f, 10.0f, 10.0f, 23.5f, 0.98f,
                true, 10.0f, glm::vec4(0.9f, 0.5f, 0.8f, 0.5f));

    glCheckError();

    //Load skybox & glow
    Skybox skybox("../images/8k_stars_milky_way.jpg");

    skyboxShader.use();
    skyboxShader.setInt("equirectangularMap", 0);

    glCheckError();

    unsigned int glowVAO, glowTexture;
    setUpGlowEffect(glowVAO, glowTexture, glowShader);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // Camera orbiting logic
        if (camera.isOrbiting) {
            glm::vec3 earthPos = glm::vec3(earth.getModelMatrix() * glm::vec4(0.0, 0.0, 0.0, 1.0));
            camera.upDateOrbit(earthPos, currentFrame);
        }

        // render
        // ClearColor magenta to notice errors
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // shared matrices / data
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 4000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::vec3 sunPos = glm::vec3(sun.getModelMatrix() * glm::vec4(0.0, 0.0, 0.0, 1.0));

        // Skybox
        skybox.Draw(skyboxShader, view, projection);

        // Draw glows
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        glowShader.use();
        glowShader.setMat4("projection", projection);
        glowShader.setMat4("view", view);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glowTexture);
        glBindVertexArray(glowVAO);

        earth.DrawGlow(glowShader, view);
        for (auto& planet: planets) {
            planet.DrawGlow(glowShader, view);
        }

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);


        // Draw sun
        sunShader.use();
        sunShader.setMat4("projection", projection);
        sunShader.setMat4("view", view);
        sun.Draw(sunShader);

        // Earth
        earthShader.use();
        earthShader.setMat4("projection", projection);
        earthShader.setMat4("view", view);
        earthShader.setVec3("lightPos", sunPos);
        earthShader.setFloat("u_time", (float)glfwGetTime());
        earth.Draw(earthShader);

        // Other planets
        planetShader.use();
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", sunPos);

        for (auto& planet : planets) {
            planet.Draw(planetShader);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // close window, terminate GLFW
    glfwTerminate();

    return 0;

}

/***********************************
 * Helper function implementations *
 * *********************************/


/**
 * @brief  Checks what keys were pressed and decides what to do with them
 *
 * @param[in] window Window to check for
 */
void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    static bool zeroPressedLastFrame = false;
    static bool onePressedLastFrame = false;

    bool zeroPressedThisFrame = glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS;
    bool onePressedThisFrame = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;

   // Use edge detection
   if (zeroPressedThisFrame && !zeroPressedLastFrame) {
       camera.isOrbiting = true;
       firstMouse = true; // Reset mouse tracking to prevent camera jump when orbit ends
   }
   if (onePressedThisFrame && !onePressedLastFrame) {
       camera.Reset(); // Resets camera to initial position and sets isOrbiting to false
   }

   zeroPressedLastFrame = zeroPressedThisFrame;
   onePressedLastFrame = onePressedThisFrame;

   if (camera.isOrbiting) {
       return;
   }

   if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
       camera.ProcessKeyboard(FORWARD, deltaTime);
   if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
       camera.ProcessKeyboard(BACKWARD, deltaTime);
   if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
       camera.ProcessKeyboard(LEFT, deltaTime);
   if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
       camera.ProcessKeyboard(RIGHT, deltaTime);

    // We need to do all of this because we want edge detection: We don't need flickering if the user holds space
    static bool spacePressedLastFrame = false;
    static bool isWireframe = false;

    bool spacePressedThisFrame = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

   if(spacePressedThisFrame && !spacePressedLastFrame) { // This is for debugging, being able to swap to and from wireframe mode

       if (isWireframe)
           glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
       else
           glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

       isWireframe = !isWireframe;
   }

   spacePressedLastFrame = spacePressedThisFrame;
}

/**
 * @brief This helper function prints only if there is an error; it is useful since by default, OpenGL only gives error codes
 *
 * @param[in] file The file in which the error occurred, this is given by the preprocessor directive
 * @param[in] line the line in which the error occurred, also given by preprocessor directive
 * @return The error code as given by glGetError()
 */
GLenum glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cerr << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}


/**
 * @brief Initializes GLFW, creates a window, sets up callbacks.
 * @return Pointer to created GLFWwindow, nullptr on failure.
 */
GLFWwindow* initWindowManager() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Just for MacOS
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE); // Fullscreen MacOS
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to open GLFW window." << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLEW
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    return window;
}

void setUpGlowEffect(unsigned int& vao, unsigned int& textureID, Shader& shader) {
    // load texture
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("../images/soft_glow.png", &width, &height, &nrChannels, 4);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    } else {
        std::cerr << "Failed to load glow texture: ../images/soft_glow.png" << std::endl;
    }
    shader.use();
    shader.setInt("glowTexture", 0);

    // --- Billboard Quad VAO ---
    float quadVertices[] = {
        // positions     // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    unsigned int vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    // Texture coord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

/********************
*  GLFW Callbacks   *
*********************/

/**
 * @brief Function to call when the window size changes so that our viewport keeps the correct size
 *
 * @param[in] window Window to change
 * @param[in] width width to which we scould change to
 * @param[in] height height to which we should change to
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {

    if (camera.isOrbiting) {
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed, y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovements(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if (camera.isOrbiting) {
        return;
    }
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
