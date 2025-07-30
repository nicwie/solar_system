#ifndef INCLUDE_SOLAR_SYSTEM_PLANET_HPP_
#define INCLUDE_SOLAR_SYSTEM_PLANET_HPP_

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Model.hpp"
#include "Shader.hpp"

/**
 * @class Planet
 * @brief Represents a celestial body with orbital and rotational properties
 *
 * This class encapsulates a 3D model and adds behavior to make it
 * act like a planet. It handles position, rotation, and scale over time.
 */
class Planet {
public:
    /**
     * @brief Constructs a Planet
     * @param modelPath Path to 3D model file.
     * @param scale Uniform scale of the planet.
     * @param orbitalRadius Average radius of the orbit.
     * @param orbitalSpeed Speed at which the planet orbits the origin.
     * @param axialSpeed Speed at which the planet rotates on its own axis.
     * @param axialTiltAngle Planetary axial tilt in degrees.
     * @param ellipticity Ratio of the orbit's minor to major axis (1.0 for a perfect circle).
     * @param hasGlow Whether the planet should have an atmospheric glow effect.
     * @param glowScale Size of the glow billboard.
     * @param glowTint Color of the glow.
     */
    Planet(const std::string& modelPath,
           float scale,
           float orbitalRadius,
           float orbitalSpeed,
           float axialSpeed,
           float axialTiltAngle,
           float ellipticity = 1.0f,
           bool hasGlow = false,
           float glowScale = 0.0f,
           glm::vec4 glowTint = glm::vec4(0.0f))
        : m_model(modelPath),
          m_scale(scale),
          m_orbitalRadius(orbitalRadius),
          m_orbitalSpeed(orbitalSpeed * 0.025), // Changed rate here
          m_axialSpeed(axialSpeed),
          m_axialTiltAngle(axialTiltAngle),
          m_ellipticity(ellipticity),
          m_hasGlow(hasGlow),
          m_glowScale(glowScale),
          m_glowTint(glowTint)
    {}

    virtual ~Planet() = default;

    // Move
    Planet (Planet&& other) noexcept = default;
    Planet& operator=(Planet&& other) noexcept = default;

    // copy
    Planet(const Planet&) = delete;
    Planet& operator=(const Planet&) = delete;


    /**
     * @brief Updates the planet's state for the current frame.
     * @param time Current application time
     *
     * This calculates the new model matrix based on the planet's orbital
     * and rotational properties
     */
    void Update(float time) {
        glm::mat4 model = glm::mat4(1.0f);

        // Orbital position
        float angle = time * m_orbitalSpeed;
        float x = m_orbitalRadius * glm::cos(angle);
        float z = m_orbitalRadius * m_ellipticity * glm::sin(angle);
        model = glm::translate(model, glm::vec3(x, 0.0f, z));

        // Axial Rotation
        model = glm::rotate(model, glm::radians(time * m_axialSpeed), glm::vec3(0.0f, 1.0f, 0.0f));

        // Axial tilt
        model = glm::rotate(model, glm::radians(m_axialTiltAngle), glm::vec3(0.0f, 0.0f, 1.0f));

        // Scale
        m_modelMatrix = glm::scale(model, glm::vec3(m_scale));
    }

    /**
     * @brief Calculates and returns the model matrix
     */
    glm::mat4 getModelMatrix() const {
        return m_modelMatrix;
    }

    /**
     * @brief Draws the planet model itself.
     * @param shader Shader to use for drawing
     *
     * This assumes Update() has already been called for the current frame
     */
    virtual void Draw(Shader& shader) {
        shader.setMat4("model", m_modelMatrix);
        m_model.Draw(shader);
    }

    /**
     * @brief Draws the glow effect if enabled.
     * @param glowShader Shader for glow billboard
     * @param view Camera view matrix
     */
    void DrawGlow(Shader& glowShader, const glm::mat4& view) {
        if (!m_hasGlow) return;

        glm::vec3 planetPos = glm::vec3(getModelMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

        // Billboard matrix
        glm::mat4 glowModelMatrix = glm::translate(glm::mat4(1.0f), planetPos);
        glowModelMatrix *= glm::transpose(glm::mat4(glm::mat3(view)));
        glowModelMatrix = glm::scale(glowModelMatrix, glm::vec3(m_glowScale));

        glowShader.setMat4("model", glowModelMatrix);
        glowShader.setVec4("glowTint", m_glowTint);

        // Actual draw call is in main loop
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

protected:
    Model m_model;
    glm::mat4 m_modelMatrix;

    float m_scale;
    float m_orbitalRadius;
    float m_orbitalSpeed;
    float m_axialSpeed;
    float m_axialTiltAngle;
    float m_ellipticity;

    // Glow properties
    bool m_hasGlow;
    float m_glowScale;
    glm::vec4 m_glowTint;

};

#endif  // INCLUDE_SOLAR_SYSTEM_PLANET_HPP_
