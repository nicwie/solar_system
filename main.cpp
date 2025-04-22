
#include <iostream>

#include <GL/glew.h>
#include <GL/gl.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>


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


// This is our vertex shader: only passes the data over, but is needed in Opengl
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

// Same for our fragment shader: This **must** be set, and here only colors whatever passes through
const char *fragmentShaderSourceTri1 = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 ourColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = ourColor;\n"
    "}\0";

const char *fragmentShaderSourceTri2 = "#version 330 core\n"
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


    // This actually compiles the vertex shader we defined above
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // This is for compile time errors in shader compilation
    int  success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShaderTri1;
    fragmentShaderTri1 = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShaderTri1, 1, &fragmentShaderSourceTri1, NULL);
    glCompileShader(fragmentShaderTri1);

    glGetShaderiv(fragmentShaderTri1, GL_COMPILE_STATUS, &success);

    if(!success) {
        glGetShaderInfoLog(fragmentShaderTri1, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT1::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int shaderProgramTri1;
    shaderProgramTri1 = glCreateProgram();

    glAttachShader(shaderProgramTri1, vertexShader);
    glAttachShader(shaderProgramTri1, fragmentShaderTri1);
    glLinkProgram(shaderProgramTri1);

    glGetProgramiv(shaderProgramTri1, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgramTri1, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM1::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Now we create a second shader for the other triangle, which we want in a different color
    unsigned int fragmentShaderTri2;
    fragmentShaderTri2 = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShaderTri2, 1, &fragmentShaderSourceTri2, NULL);
    glCompileShader(fragmentShaderTri2);

    glGetShaderiv(fragmentShaderTri2, GL_COMPILE_STATUS, &success);

    if(!success) {
        glGetShaderInfoLog(fragmentShaderTri2, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER:FRAGMENT2::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int shaderProgramTri2;
    shaderProgramTri2 = glCreateProgram();

    glAttachShader(shaderProgramTri2, vertexShader);
    glAttachShader(shaderProgramTri2, fragmentShaderTri2);
    glLinkProgram(shaderProgramTri2);

    glGetProgramiv(shaderProgramTri2, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgramTri2, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM2::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Now that we have compiled and linked, we do not need the shaders anymore
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShaderTri1);
    glDeleteShader(fragmentShaderTri2);

    // test triangle
    float verticesTri1[] = {
         0.5f,  0.5f, 0.0f,  // top right, triangle 1
         0.5f, -0.5f, 0.0f,  // bottom right, triangle 1
        -0.5f, -0.5f, 0.0f,  // bottom left, triangle 1
    };

    float verticesTri2[] = {
        -0.6f,  0.5f, 0.0f,  // top left, triangle 2
        -0.6f, -0.5f, 0.0f,  // bottom left, triangle 2
         0.4f,  0.5f, 0.0f   // top right, triangle 2
    };

    GLuint VBOTri1, VAOTri1;
    GLuint VBOTri2, VAOTri2;

    glGenVertexArrays(1, &VAOTri1);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAOTri1);

    glGenBuffers(1, &VBOTri1);
    glBindBuffer(GL_ARRAY_BUFFER, VBOTri1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesTri1), verticesTri1, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    glGenVertexArrays(1, &VAOTri2);

    glBindVertexArray(VAOTri2);

    glGenBuffers(1, &VBOTri2);
    glBindBuffer(GL_ARRAY_BUFFER, VBOTri2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesTri2), verticesTri2, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // main drawing loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgramTri1);
        
        // Use Tri1 Shader uniform to change color over time
        float timeValue = glfwGetTime();
        float greenValue = sin(timeValue) / 2.0f + 0.5f;
        int vertexColorLocation = glGetUniformLocation(shaderProgramTri1, "ourColor");
        glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

        glBindVertexArray(VAOTri2);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glUseProgram(shaderProgramTri2);
        glBindVertexArray(VAOTri1);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
        glCheckError();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &VAOTri1);
    glDeleteVertexArrays(1, &VAOTri2);
    glDeleteBuffers(1, &VBOTri1);
    glDeleteBuffers(1, &VBOTri2);
    glDeleteProgram(shaderProgramTri1);
    glDeleteProgram(shaderProgramTri2);

    // close window, terminate GLFW
    glfwTerminate();

    return 0;

}

