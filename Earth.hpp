#ifndef INCLUDE_SOLAR_SYSTEM_EARTH_HPP_
#define INCLUDE_SOLAR_SYSTEM_EARTH_HPP_

#include <string>

#include "Model.hpp"
#include "Planet.hpp"
#include "Shader.hpp"

class Earth : public Planet {
public:
    Earth(const std::string& ModelPath, const std::string& dayTexturePath, const std::string& nightTexturePath, const std::string& cloudTexturePath,const std::string& specularMapPath,float scale, float orbitalRadius, float orbitalSpeed, float axialSpeed, float axialTiltAngle, float ellipticity,
           bool hasGlow = false, float glowScale = 0.0f, glm::vec4 glowTint = glm::vec4(0.0f))
        : Planet(ModelPath, scale, orbitalRadius, orbitalSpeed, axialSpeed, axialTiltAngle, hasGlow, glowScale, glowTint, ellipticity)
    {
        p_dayTextureID = model.loadTexture(dayTexturePath);
        p_nightTextureID = model.loadTexture(nightTexturePath);
        p_cloudTextureID = model.loadTexture(cloudTexturePath);
        p_specularMapTextureID = model.loadTexture(specularMapPath);
    };

    void Draw(Shader& shader) override {
        // 1. Set the uniforms that this shader needs
        glm::mat4 modelMatrix = getModelMatrix();
        shader.setMat4("model", modelMatrix);
        shader.setInt("texture_day", 0);
        shader.setInt("texture_night", 1);
        shader.setInt("texture_clouds", 2);
        shader.setInt("texture_specMap", 3);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, p_dayTextureID);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, p_nightTextureID);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, p_cloudTextureID);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, p_specularMapTextureID);

        this->model.Draw();

        glActiveTexture(GL_TEXTURE0);
    }

private:
    unsigned int p_nightTextureID;
    unsigned int p_dayTextureID;
    unsigned int p_cloudTextureID;
    unsigned int p_specularMapTextureID;
};

#endif  // INCLUDE_SOLAR_SYSTEM_EARTH_HPP_
