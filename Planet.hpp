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
    Planet(const std::string& modelPath,
           float scale,
           float orbitalRadius,
           float orbitalSpeed,
           float axialSpeed,
           float axialTiltAngle,
           bool hasGlow = false,
           float glowScale = 0.0f,
           glm::vec4 glowTint = glm::vec4(0.0f),
           float ellipticity = 1.0f) // <-- neu
        : model(modelPath),
          p_Scale(scale),
          p_OrbitalRadius(orbitalRadius),
          p_OrbitalSpeed(orbitalSpeed),
          p_AxialSpeed(axialSpeed),
          p_AxialTiltAngle(axialTiltAngle),
          p_hasGlow(hasGlow),
          p_glowScale(glowScale),
          p_glowTint(glowTint),
          p_Ellipticity(ellipticity) {} // <-- neu

    /**
     * @brief Calculates and returns the model matrix
     */
    glm::mat4 getModelMatrix() {
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        // Zeitabhängiger Winkel (Bogenmaß)
        float angle = static_cast<float>(glfwGetTime()) * p_OrbitalSpeed;

        // Elliptische Umlaufbahn
        float x = p_OrbitalRadius * glm::cos(angle);
        float z = p_OrbitalRadius * p_Ellipticity * glm::sin(angle);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(x, 0.0f, z));

        // Axial Tilt (z.B. Erdneigung 23.5°)
        modelMatrix = glm::rotate(modelMatrix, glm::radians(p_AxialTiltAngle), glm::vec3(0.0f, 0.0f, 1.0f));

        // Rotation um eigene Achse (Tagesrotation)
        modelMatrix = glm::rotate(modelMatrix,
                                  glm::radians(static_cast<float>(glfwGetTime()) * p_AxialSpeed),
                                  glm::vec3(0.0f, 1.0f, 0.0f));

        // Skalierung des Planeten
        modelMatrix = glm::scale(modelMatrix, glm::vec3(p_Scale));

        return modelMatrix;
    }

    /**
     * @brief Draws the planet model itself.
     */
    void Draw(Shader& shader) {
        glm::mat4 modelMatrix = getModelMatrix();
        shader.setMat4("model", modelMatrix);
        this->model.Draw(shader);
    }

    /**
     * @brief Draws the glow effect if enabled.
     */
    void DrawGlow(Shader& glowShader, const glm::mat4& view) {
        if (!p_hasGlow) return;

        // Weltposition des Planeten bestimmen
        glm::vec3 planetPos = glm::vec3(getModelMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

        // Billboard-Matrix: Position + Ansicht aus View-Matrix extrahiert
        glm::mat4 glowModelMatrix = glm::translate(glm::mat4(1.0f), planetPos);
        glowModelMatrix *= glm::transpose(glm::mat4(glm::mat3(view))); // Nur Rotation
        glowModelMatrix = glm::scale(glowModelMatrix, glm::vec3(p_glowScale));

        glowShader.setMat4("model", glowModelMatrix);
        glowShader.setVec4("glowTint", p_glowTint);

        // Zeichnen (Quad in Hauptprogramm setzen)
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

    float p_Ellipticity; // Verhältnis b/a der Ellipse (z. B. 0.8)
};

#endif  // INCLUDE_SOLAR_SYSTEM_PLANET_HPP_
