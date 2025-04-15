
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

using namespace glm;

/**
 * @brief  Checks what keys were pressed and decides what to do with them
 *
 * @param[[TODO:direction]] window [TODO:description]
 */
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

/**
 * @brief Function to call when the window size changes so that our vieport keeps the correct size
 *
 * @param[[TODO:direction]] window [TODO:description]
 * @param[[TODO:direction]] width [TODO:description]
 * @param[[TODO:direction]] height [TODO:description]
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
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Just for MacOS
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

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, 800, 600);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// to be able to capture press <esc>
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Background color (here , Black)
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);

	// Accept if closer to camer than other
	glDepthFunc(GL_LESS);

	// main drawing loop 
	while (!glfwWindowShouldClose(window)) {
		processInput(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	// main drawing loop
	/* do {
		// clear screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	  	glfwWindowShouldClose(window) == 0);
	*/

	// close window, terminate GLFW
	glfwTerminate();

	return 0;

}
