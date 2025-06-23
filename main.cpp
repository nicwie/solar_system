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

float mixVal = 0.2f;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
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

    static bool upKeyLastFrame = false;
    static bool downKeyLastFrame = false;

    bool spacePressedThisFrame = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    bool upKeyThisFrame = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
    bool downKeyThisFrame = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;

    if(spacePressedThisFrame && !spacePressedLastFrame) { // This is for debugging, being able to swap to and from wireframe mode

        if (isWireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        isWireframe = !isWireframe;
    }

    if (upKeyThisFrame && !upKeyLastFrame && mixVal <= 1.0f) {
        mixVal += 0.1;
    }

    if (downKeyThisFrame && !downKeyLastFrame && mixVal >= -1.0f) {
        mixVal -= 0.1;
    }

    spacePressedLastFrame = spacePressedThisFrame;
    upKeyLastFrame = upKeyThisFrame;
    downKeyLastFrame = downKeyThisFrame;
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

    Shader planetShader("../lighting_planet.vs", "../lighting_planet.fs");
    Shader sunShader("../lighting_planet.vs", "../lighting_sun.fs");
    glCheckError();

    Planet sun("../Sun_1_1391000.glb", 10.0f, 0.0f, 0.0f, 2.0f);
    Planet earth("../Earth_1_12756.glb", 0.01f, 50.0f, 10.0f, 10.0f);
    //Planet moon("../Moon_1_3474.glb", 0.002f, 0.0f, 0.0f, 2.0f);
    Planet mars("../24881_Mars_1_6792.glb", 0.005f, 100.0f, 20.0f, 10.0f);
    Planet venus("../Venus_1_12103.glb", 0.008f, 30.0f, 5.0f, 10.0f);
    Planet jupiter("../Jupiter_1_142984.glb", 0.05f, 200.0f, 30.0f, 10.0f);
    Planet saturn("../Saturn_1_120536.glb", 0.04f, 150.0f, 25.0f, 10.0f);
    Planet uranus("../Uranus_1_51118.glb", 0.02f, 250.0f, 40.0f, 10.0f);
    Planet neptune("../Neptune_1_49528.glb", 0.02f, 300.0f, 50.0f, 10.0f);
    Planet merkur("../Mercury_1_4878.glb", 0.003f, 20.0f, 2.0f, 10.0f);

    glCheckError();

    Shader skyboxShader("../skybox.vs", "../skybox.fs");

    glCheckError();

    Skybox skybox("../8k_stars_milky_way.jpg");

    skyboxShader.use();
    skyboxShader.setInt("equirectangularMap", 0);

    glCheckError();

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


        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // Skybox
        skyboxShader.use();
        skybox.Draw(skyboxShader, view, projection);

        // Draw Sun
        sunShader.use();

        // view/projection transformations
        sunShader.setMat4("projection", projection);
        sunShader.setMat4("view", view);

        sun.Draw(sunShader);


        glm::mat4 sunModelMatrix = sun.getModelMatrix();

        glm::vec3 sunPos = glm::vec3(sunModelMatrix * glm::vec4(0.0, 0.0, 0.0, 1.0));

        // Earth
        planetShader.use();

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
        planetShader.use();

        // view/projection transformations
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", sunPos);

        venus.Draw(planetShader); 
        
        // Merkur
        planetShader.use();

        // view/projection transformations
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", sunPos);

        merkur.Draw(planetShader);      

        // Jupiter
        planetShader.use();

        // view/projection transformations
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", sunPos);

        jupiter.Draw(planetShader);   

        // Saturn
        planetShader.use();

        // view/projection transformations
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", sunPos);

        saturn.Draw(planetShader);   

        // Uranus
        planetShader.use();

        // view/projection transformations
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightPos", sunPos);

        uranus.Draw(planetShader);   

        // Neptune
        planetShader.use();

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

