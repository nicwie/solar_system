#ifndef INCLUDE_SOLAR_SYSTEM_EARTH_HPP_
#define INCLUDE_SOLAR_SYSTEM_EARTH_HPP_

#include <string>

#include "Model.hpp"
#include "Planet.hpp"
#include "Shader.hpp"

class Earth : public Planet {
public:
    Earth(unsigned int dayTexID, const std::string& ModelPath, const std::string& dayTexturePath, const std::string& nightTexturePath, const std::string& cloudTexturePath, float scale, float orbitalRadius, float orbitalSpeed, float axialSpeed, float axialTiltAngle,
           bool hasGlow = false, float glowScale = 0.0f, glm::vec4 glowTint = glm::vec4(0.0f))
        : Planet(ModelPath, scale, orbitalRadius, orbitalSpeed, axialSpeed, axialTiltAngle, hasGlow, glowScale, glowTint) 
    {
        p_nightTextureID = model.loadTexture(nightTexturePath);
        // p_dayTextureID = model.loadTexture(dayTexturePath);
        p_dayTextureID = dayTexID;
    };

    // void Draw(Shader& shader) override {
    //     shader.setInt("texture_day", 0);
    //     shader.setInt("texture_night", 1);
    //
    //     glActiveTexture(GL_TEXTURE0);
    //     glBindTexture(GL_TEXTURE_2D, p_dayTextureID);
    //
    //     glActiveTexture(GL_TEXTURE1);
    //     glBindTexture(GL_TEXTURE_2D, p_nightTextureID);
    //
    //     Planet::Draw(shader);
    // }
    
    // void Draw(Shader& shader) override {
    //
    //     glm::mat4 modelMatrix = getModelMatrix();
    //     shader.setMat4("model", modelMatrix);
    //
    //     shader.setInt("texture_day", 0);
    //     shader.setInt("texture_night", 1);
    //
    //     glActiveTexture(GL_TEXTURE0);
    //     glBindTexture(GL_TEXTURE_2D, p_dayTextureID);
    //
    //     glActiveTexture(GL_TEXTURE1);
    //     glBindTexture(GL_TEXTURE_2D, p_nightTextureID);
    //
    //     this->model.Draw();
    // }

    // void Draw(Shader& shader) override {
    //     // 1. Set the uniforms that this shader needs
    //     glm::mat4 modelMatrix = getModelMatrix();
    //     shader.setMat4("model", modelMatrix);
    //     shader.setInt("texture_day", 0);
    //     shader.setInt("texture_night", 1);
    //     
    //     // 2. Bind our two custom external textures to their correct texture units
    //     glActiveTexture(GL_TEXTURE0);
    //     glBindTexture(GL_TEXTURE_2D, p_dayTextureID);
    //
    //     glActiveTexture(GL_TEXTURE1);
    //     glBindTexture(GL_TEXTURE_2D, p_nightTextureID);
    //
    //     // 3. Now, tell the underlying "dumb" model to draw its geometry.
    //     // We do NOT call Planet::Draw(), as that would cause the texture conflict.
    //     this->model.Draw();
    //
    //     // 4. It's good practice to reset the active texture unit back to 0 when we're done.
    //     glActiveTexture(GL_TEXTURE0);
    // }


    void Draw(Shader& shader) override {
        // This is a minimal, hardcoded draw call for this specific test.
        glm::mat4 modelMatrix = getModelMatrix();

        // 1. Set the model matrix
        shader.setMat4("model", modelMatrix);

        // 2. Tell the shader that our simple texture will be on texture unit 0
        shader.setInt("texture_day", 0);
        
        // 3. Bind ONLY the day texture to unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, p_dayTextureID);

        // 4. Draw the model's geometry
        this->model.Draw();

        // 5. Unbind the texture to be safe (good practice)
        glBindTexture(GL_TEXTURE_2D, 0);
    }

private:
    unsigned int p_nightTextureID;
    unsigned int p_dayTextureID;
    unsigned int p_cloudTextureID;
};

#endif  // INCLUDE_SOLAR_SYSTEM_EARTH_HPP_
