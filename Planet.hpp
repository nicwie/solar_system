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
    Planet(const std::string& modelPath, float scale, float orbitalRadius, float orbitalSpeed, float axialSpeed, float axialTiltAngle,
           bool hasGlow = false, float glowScale = 0.0f, glm::vec4 glowTint = glm::vec4(0.0f))
    : model(modelPath),
      p_Scale(scale),
      p_OrbitalRadius(orbitalRadius),
      p_OrbitalSpeed(orbitalSpeed),
      p_AxialSpeed(axialSpeed),
      p_AxialTiltAngle(axialTiltAngle),
      p_hasGlow(hasGlow),
      p_glowScale(glowScale),
      p_glowTint(glowTint) {}

    /**
     * @brief Calculates and returns the model matrix
     *
     */
    glm::mat4 getModelMatrix() {
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        // Orbital Rotation
        modelMatrix = glm::rotate(modelMatrix,
                                  glm::radians((float)glfwGetTime() * p_OrbitalSpeed),
                                  glm::vec3(0.0f, 1.0f, 0.0f));

        // Translation
        modelMatrix = glm::translate(modelMatrix, glm::vec3(p_OrbitalRadius, 0.0f, 0.0f));

        // Axial Tilt
        // We tilt around the Z-Axis
        modelMatrix = glm::rotate(modelMatrix, glm::radians(p_AxialTiltAngle), glm::vec3(0.0f,0.0f,1.0f));

        // Daily Spin
        modelMatrix = glm::rotate(modelMatrix,
                                  glm::radians((float)glfwGetTime() * p_AxialSpeed),
                                  glm::vec3(0.0f, 1.0f, 0.0f));

        // Scale
        modelMatrix = glm::scale(modelMatrix, glm::vec3(p_Scale));

        return modelMatrix;
    }

    /**
     * @brief Draws the planet model itself.
     *
     * @param shader Shader used which accepts "model" and is used to draw the model
     */
    void Draw(Shader& shader) {
        glm::mat4 modelMatrix = getModelMatrix();
        shader.setMat4("model", modelMatrix);
        this->model.Draw(shader);
    }

    void DrawGlow(Shader& glowShader, const glm::mat4& view) {
        if (!p_hasGlow) return;

        // Billboard that always faces the camera
        glm::vec3 planetPos = glm::vec3(getModelMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        glm::mat4 glowModelMatrix = glm::translate(glm::mat4(1.0f), planetPos);
        glowModelMatrix = glowModelMatrix * glm::transpose(glm::mat4(glm::mat3(view)));
        glowModelMatrix = glm::scale(glowModelMatrix, glm::vec3(p_glowScale));

        glowShader.setMat4("model", glowModelMatrix);
        glowShader.setVec4("glowTint", p_glowTint);

        // Draw itself happens in main loop
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

private:
    Model model;

    float p_Scale;
    float p_OrbitalRadius;
    float p_OrbitalSpeed;
    float p_AxialSpeed;
    float p_AxialTiltAngle;

    // Glow properties
    bool p_hasGlow;
    float p_glowScale;
    glm::vec4 p_glowTint;

};

#endif  // INCLUDE_SOLAR_SYSTEM_PLANET_HPP_
