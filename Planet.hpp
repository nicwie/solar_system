#ifndef INCLUDE_SOLAR_SYSTEM_PLANET_HPP_
#define INCLUDE_SOLAR_SYSTEM_PLANET_HPP_

#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Model.hpp"
#include "Shader.hpp"

#include <GLFW/glfw3.h>

class Planet {
public:
    Planet(const std::string& modelPath, float scale, float orbitalRadius, float orbitalSpeed, float axialSpeed) 
    : model(modelPath),
      pScale(scale),
      pOrbitalRadius(orbitalRadius),
      pOrbitalSpeed(orbitalSpeed),
      pAxialSpeed(axialSpeed) {}

    void Draw(Shader& shader) {
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        modelMatrix = glm::rotate(modelMatrix,
                                  glm::radians((float)glfwGetTime() * pOrbitalSpeed),
                                  glm::vec3(0.0f, 1.0f, 0.0f));

        modelMatrix = glm::translate(modelMatrix, glm::vec3(pOrbitalRadius, 0.0f, 0.0f));

        modelMatrix = glm::rotate(modelMatrix, 
                                  glm::radians((float)glfwGetTime() * pAxialSpeed), 
                                  glm::vec3(0.0f, 1.0f, 0.0f));

        modelMatrix = glm::scale(modelMatrix, glm::vec3(pScale));

        shader.setMat4("model", modelMatrix);
        this->model.Draw(shader);
    }

private:
    Model model;

    float pScale; // The planet's scale, this will probably be the same for all planets
    float pOrbitalRadius; // How far the planet orbits from the center
    float pOrbitalSpeed;
    float pAxialSpeed;

};

#endif  // INCLUDE_SOLAR_SYSTEM_PLANET_HPP_
