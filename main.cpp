
#include <iostream>

#include <GL/glew.h>
#include <GL/gl.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include "Shader.hpp"


/**
 * @brief This helper function prints only if there is an error; this is useful since by default, openGL only gives error codes
 *
 * @param[in] file The file in which the error occured, this is given by the preprocessor directive
 * @param[in] line the line in which the error occured, also given by preprocessor directive
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


const char *vertexShaderSourceTri1 = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

// Same for our fragment shader: This **must** be set, and here only colors whatever passes through
const char *fragmentShaderSourceTri1 = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(0.2f, 0.5f, 1.0f, 1.0f);\n"
    "}\0";

/**
 * @brief  Checks what keys were pressed and decides what to do with them
 *
 * @param[in] window Window to check for
 */
void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // We need to do all of this because we want edge detection: We don't need flickering if the user holds space
    static bool spacePressedLastFrame = false;
    static bool isWireframe = false;

    bool spacePressedThisFrame = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    if(spacePressedThisFrame && !spacePressedLastFrame) { // This is for debugging, being able to swap to and from wireframe mode

        if (isWireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        isWireframe = !isWireframe;
    }

    spacePressedLastFrame = spacePressedThisFrame;
}

/**
 * @brief Function to call when the window size changes so that our vieport keeps the correct size
 *
 * @param[in] window Window to change
 * @param[in] width width to which we schould change to
 * @param[in] height height to which we should change to
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main(void) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initalize GLFW\n");
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
    GLFWwindow* window = glfwCreateWindow(800, 600,  "Solar System", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, 800, 600);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Background color (here , Black)
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Creating a shader
    Shader Tri("../horizontalOffset.vs", "../passColor.fs");

    // this triangle has colors as well
    float verticesTri[] = {
        // position          //colors
        -0.5f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f, // top left, triangle 2
         0.0f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f, // bottom left, triangle 2
         0.5f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f  // top right, triangle 2
    };

    GLuint VBOTri, VAOTri;

    glCheckError();

    glGenVertexArrays(1, &VAOTri);

    glBindVertexArray(VAOTri);

    glGenBuffers(1, &VBOTri);
    glBindBuffer(GL_ARRAY_BUFFER, VBOTri);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesTri), verticesTri, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    // We need this to point to the second vector of colors
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    glCheckError();

    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // main drawing loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        float offsetVal = (sin(glfwGetTime()));

        Tri.setFloat("offset", offsetVal);

        Tri.use();
        glBindVertexArray(VAOTri);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
        glCheckError();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &VAOTri);
    glDeleteBuffers(1, &VBOTri);

    // close window, terminate GLFW
    glfwTerminate();

    return 0;

}

