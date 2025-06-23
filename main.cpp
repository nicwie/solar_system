#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>

#include <GL/glew.h>
#include <GL/gl.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <ostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.hpp"
#include "Shader.hpp"
#include "Planet.hpp"
#include "Skybox.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// settings
const unsigned int SCR_WIDTH = 800;
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

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Background color (here , Black)
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

    Planet sun("../models/Sun_1_1391000.glb", 50.0f, 0.0f, 0.0f, 1.0f);
    Planet mercury("../models/Mercury_1_4878.glb", 0.0038f, AU * 0.39f, 42.0f, 10.0f);
    Planet venus("../models/Venus_1_12103.glb", 0.0095f, AU * 0.72f, 16.0f, 10.0f);
    Planet earth("../models/Earth_1_12756.glb", 0.01f, AU * 1.00f, 10.0f, 10.0f);
    Planet mars("../models/24881_Mars_1_6792.glb", 0.0053f, AU * 1.52f, 5.0f, 10.0f);
    Planet jupiter("../models/Jupiter_1_142984.glb", 0.112f, AU * 5.20f, 1.0f, 10.0f);
    Planet saturn("../models/Saturn_1_120536.glb", 0.093f, AU * 9.58f, 0.6f, 10.0f);
    Planet uranus("../models/Uranus_1_51118.glb", 0.04f, AU * 19.2f, 0.2f, 10.0f);
    Planet neptune("../models/Neptune_1_49528.glb", 0.038f, AU * 30.1f, 0.1f, 10.0f);



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
    unsigned char *data = stbi_load("../images/soft_glow.png", &width, &height, &nrChannels, 0);

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

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 3000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // Skybox
        skyboxShader.use();
        skybox.Draw(skyboxShader, view, projection);

        // Render Earth Glow
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        glowShader.use();

        glm::mat4 earthModelMatrix = earth.getModelMatrix();
        glm::vec3 earthPos = glm::vec3(earthModelMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

        glm::mat4 glowModelMatrix = glm::translate(glm::mat4(1.0f), earthPos);

        glowModelMatrix = glowModelMatrix * glm::transpose(glm::mat4(glm::mat3(view)));

        glowModelMatrix = glm::scale(glowModelMatrix, glm::vec3(8.0f));

        glowShader.setMat4("model", glowModelMatrix);

        // Bind the glow texture and draw the quad
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glowTexture);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glowShader.setMat4("projection", projection);
        glowShader.setMat4("view", view);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        // Draw Sun
        sunShader.use();

        // view/projection transformations
        sunShader.setMat4("projection", projection);
        sunShader.setMat4("view", view);

        sun.Draw(sunShader);


        glm::mat4 sunModelMatrix = sun.getModelMatrix();

        glm::vec3 sunPos = glm::vec3(sunModelMatrix * glm::vec4(0.0, 0.0, 0.0, 1.0));

        // Earth
        earthShader.use();

        // view/projection transformations
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", sunPos);

        earth.Draw(planetShader);

        // Mars
        planetShader.use();

        // view/projection transformations
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", sunPos);

        mars.Draw(planetShader);

        // Venus
        // view/projection transformations
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", sunPos);

        venus.Draw(planetShader);

        // Merkur
        // view/projection transformations
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", sunPos);

        mercury.Draw(planetShader);

        // Jupiter
        // view/projection transformations
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", sunPos);

        jupiter.Draw(planetShader);

        // Saturn
        // view/projection transformations
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", sunPos);

        saturn.Draw(planetShader);

        // Uranus
        // view/projection transformations
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", sunPos);

        uranus.Draw(planetShader);

        // Neptune
        // view/projection transformations
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", sunPos);

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

