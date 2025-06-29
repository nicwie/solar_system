#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>

#include <GL/glew.h>
#ifdef __APPLE__
    #include <OpenGL/gl.h> // Just for macOS
#else
    #include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <ostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.hpp"
#include "Shader.hpp"
#include "Planet.hpp"
#include "Earth.hpp"
#include "Skybox.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// settings
const unsigned int SCR_WIDTH = 960;
const unsigned int SCR_HEIGHT = 600;

// camera, with a starting position looking down
Camera camera(glm::vec3(0.0f, 60.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -89.0f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

/**
 * @brief This helper function prints only if there is an error; it is useful since by default, openGL only gives error codes
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
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)


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

   // --- NEW: If camera is orbiting, do not process any other movement input ---
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

int main(void) {

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Just for MacOS
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE); // Fullscreen MacOS
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open the window itself
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,  "Solar System", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.");
        getchar();
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // This is done so that textures are rendered according to their z distance from us
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Background color (here, black)
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glCheckError();

    // From here: setting up objects

    // Earth atmosphere has some unique properties so we try to make it look cool
    Shader earthShader("../shaders/lighting_earth.vs", "../shaders/lighting_earth.fs");
    Shader planetShader("../shaders/lighting_planet.vs", "../shaders/lighting_planet.fs");
    Shader sunShader("../shaders/lighting_planet.vs", "../shaders/lighting_sun.fs");
    Shader skyboxShader("../shaders/skybox.vs", "../shaders/skybox.fs");
    glCheckError();

    float AU = 120.0f; // Astronomical Unit, used to scale the solar system


    Planet sun("../models/Sun_1_1391000.glb", 50.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // Planeten mit elliptischer Umlaufbahn (letzter Parameter = ellipticity)
    Planet mercury("../models/Mercury_1_4878.glb", 0.0038f, AU * 0.39f, 42.0f, 10.0f, 0.03f,
                false, 0.0f, glm::vec4(0.0f), 0.8f);  // sehr elliptisch

    Planet venus("../models/Venus_1_12103.glb", 0.0095f, AU * 0.72f, 16.0f, 10.0f, 177.4f,
                false, 0.0f, glm::vec4(0.0f), 0.95f);  // fast kreisförmig

    Earth earth("../models/earth(1).glb", "../images/2k_earth_daymap.jpg", "../images/2k_earth_nightmap.jpg", "../images/2k_earth_clouds.jpg", 4.01f, AU * 1.0f, 10.0f, 10.0f, 23.5f, 0.98f,
                 true, 10.0f, glm::vec4(0.9f, 0.5f, 0.8f, 0.5f));

    Planet mars("../models/24881_Mars_1_6792.glb", 0.0053f, AU * 1.52f, 5.0f, 10.0f, 25.2f,
                true, 5.0f, glm::vec4(0.9f, 0.4f, 0.2f, 0.4f), 0.92f);

    Planet jupiter("../models/Jupiter_1_142984.glb", 0.112f, AU * 5.20f, 1.0f, 10.0f, 3.1f,
                false, 0.0f, glm::vec4(0.0f), 0.96f);

    Planet saturn("../models/Saturn_1_120536.glb", 0.093f, AU * 9.58f, 0.6f, 10.0f, 26.7f,
                false, 0.0f, glm::vec4(0.0f), 0.95f);

    Planet uranus("../models/Uranus_1_51118.glb", 0.04f, AU * 19.2f, 0.2f, 10.0f, 97.8f,
                false, 0.0f, glm::vec4(0.0f), 0.94f);

    Planet neptune("../models/Neptune_1_49528.glb", 0.038f, AU * 30.1f, 0.1f, 10.0f, 28.3f,
               false, 0.0f, glm::vec4(0.0f), 0.96f);




    glCheckError();


    glCheckError();

    Skybox skybox("../images/8k_stars_milky_way.jpg");

    skyboxShader.use();
    skyboxShader.setInt("equirectangularMap", 0);

    glCheckError();

    // This shader creates a "glow" from the earth's atmosphere
    Shader glowShader("../shaders/glow.vs", "../shaders/glow.fs");

    unsigned int glowTexture;

    glGenTextures(1, &glowTexture);
    glBindTexture(GL_TEXTURE_2D, glowTexture);

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
        std::cerr << "Failed to load glow texture" << std::endl;
        stbi_image_free(data);
    }
    glowShader.use();
    glowShader.setInt("glowTexture", 0);

    // VAO for billboard glow quad
    float quadVertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // main drawing loop
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // Camera orbiting logic
        // -----
        if (camera.isOrbiting) {
            // Get the Earth current world position
            glm::vec3 earthPos = glm::vec3(earth.getModelMatrix() * glm::vec4(0.0, 0.0, 0.0, 1.0));

            // Define orbit parameters
            float orbitRadius = 15.0f;  // How far from the Earth to orbit
            float orbitSpeed  = 0.2f;   // How fast to orbit
            float orbitHeight = 1.0f;   // How high above the Earth's equator to be

            // Calculate the new camera position
            camera.Position.x = earthPos.x + orbitRadius * cos(currentFrame * orbitSpeed);
            camera.Position.z = earthPos.z + orbitRadius * sin(currentFrame * orbitSpeed);
            camera.Position.y = earthPos.y + orbitHeight;

            camera.lookAt(earthPos);
        }

        // render
        // ------
        // ClearColor magenta to notice errors
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // shared matrices / data
        // -----
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 4000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 sunModelMatrix = sun.getModelMatrix();
        glm::vec3 sunPos = glm::vec3(sunModelMatrix * glm::vec4(0.0, 0.0, 0.0, 1.0));

        // Skybox
        // ------
        skybox.Draw(skyboxShader, view, projection); // Shader is activated in Draw func

        // Draw glows
        // ------
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        glowShader.use();
        glowShader.setMat4("projection", projection);
        glowShader.setMat4("view", view);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glowTexture);
        glBindVertexArray(quadVAO);

        earth.DrawGlow(glowShader, view);
        mars.DrawGlow(glowShader, view);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);


        // Draw sun
        // ------
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

        mercury.Draw(planetShader);
        venus.Draw(planetShader);
        mars.Draw(planetShader);
        jupiter.Draw(planetShader);
        saturn.Draw(planetShader);
        uranus.Draw(planetShader);
        neptune.Draw(planetShader);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // close window, terminate GLFW
    glfwTerminate();

    return 0;

}

