#ifndef INCLUDE_SOLAR_SYSTEM_SKYBOX_HPP_
#define INCLUDE_SOLAR_SYSTEM_SKYBOX_HPP_

#include <string>
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.hpp"

#include "stb_image.h"

/**
 * @class Skybox
 * @brief Manages the rendering of a skybox using a single panoramic (equirectangular) texture.
 *
 * This class sets up a cube mesh and loads a 2D texture that is mapped onto the
 * inside of the cube by the shader.
 */
class Skybox {
public:
    /**
     * @brief Constructs the Skybox object.
     * @param panoramicTexturePath Path to the panoramic (equirectangular) texture image.
     */
    Skybox(const std::string& panoramicTexturePath) {
        setupSkyboxMesh();
        m_panoramicTexture = loadPanoramicTexture(panoramicTexturePath);
    }

    /**
     * @brief Destructor that cleans up the VAO, VBO, and texture from the GPU.
     */
    ~Skybox() {
        glDeleteVertexArrays(1, &m_skyboxVAO);
        glDeleteBuffers(1, &m_skyboxVBO);
        glDeleteTextures(1, &m_panoramicTexture);
    }

    // Move
    Skybox(Skybox&& other) noexcept
        : m_skyboxVAO(other.m_skyboxVAO), m_skyboxVBO(other.m_skyboxVBO), m_panoramicTexture(other.m_panoramicTexture)
    {
        other.m_skyboxVAO = 0;
        other.m_skyboxVBO = 0;
        other.m_panoramicTexture = 0;
    }
    Skybox& operator=(Skybox&& other) noexcept {
        if (this != &other) {
            glDeleteVertexArrays(1, &m_skyboxVAO);
            glDeleteBuffers(1, &m_skyboxVBO);
            glDeleteTextures(1, &m_panoramicTexture);

            m_skyboxVAO = other.m_skyboxVAO;
            m_skyboxVBO = other.m_skyboxVBO;
            m_panoramicTexture = other.m_panoramicTexture;

            other.m_skyboxVAO = 0;
            other.m_skyboxVBO = 0;
            other.m_panoramicTexture = 0;
        }
        return *this;
    }

    // Copy
    Skybox(const Skybox&) = delete;
    Skybox& operator=(const Skybox&) = delete;

    /**
     * @brief Draws the skybox.
     * @param skyboxShader The shader program to use for rendering the skybox.
     * @param view The camera's current view matrix.
     * @param projection The camera's current projection matrix.
     */
    void Draw(Shader& skyboxShader, const glm::mat4& view, const glm::mat4& projection) {
        // Change depth function so depth test passes when values are equal to depth buffer's content
        glDepthFunc(GL_LEQUAL);

        skyboxShader.use();

        // Remove the translation component from the view matrix so the skybox
        // follows the camera without moving closer or farther away.
        glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
        skyboxShader.setMat4("view", viewNoTranslation);
        skyboxShader.setMat4("projection", projection);

        // Render the skybox cube
        glBindVertexArray(m_skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_panoramicTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // Set depth function back to default
        glDepthFunc(GL_LESS);
    }

private:
    unsigned int m_panoramicTexture;
    unsigned int m_skyboxVAO = 0;
    unsigned int m_skyboxVBO = 0;

    /**
     * @brief Sets up the VAO and VBO for the skybox cube.
     */
    void setupSkyboxMesh() {
        float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
        };

        glGenVertexArrays(1, &m_skyboxVAO);
        glGenBuffers(1, &m_skyboxVBO);

        glBindVertexArray(m_skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glBindVertexArray(0);
    }

    /**
     * @brief Loads a 2D texture from a file.
     * @param path The path to the texture image.
     * @return The OpenGL ID of the loaded texture.
     */
    unsigned int loadPanoramicTexture(const std::string& path) {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrChannels;
        // The skybox texture should not be flipped.
        stbi_set_flip_vertically_on_load(false);
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
        stbi_set_flip_vertically_on_load(true); // Set back to default for other models

        if (data) {
            GLenum format;
            if (nrChannels == 1)      format = GL_RED;
            else if (nrChannels == 3) format = GL_RGB;
            else if (nrChannels == 4) format = GL_RGBA;
            else {
                std::cerr << "Skybox texture at " << path << " has unsupported channel count: " << nrChannels << std::endl;
                stbi_image_free(data);
                glDeleteTextures(1, &textureID);
                return 0;
            }

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            return textureID;
        } else {
            std::cerr << "Panoramic texture failed to load at path: " << path << std::endl;
            glDeleteTextures(1, &textureID); // Clean up on failure
            return 0;
        }
    }
};

#endif  // INCLUDE_SOLAR_SYSTEM_SKYBOX_HPP_
