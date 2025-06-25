#ifndef INCLUDE_SOLAR_SYSTEM_EARTH_HPP_
#define INCLUDE_SOLAR_SYSTEM_EARTH_HPP_

#include <string>

#include "Model.hpp"
#include "Planet.hpp"
#include "Shader.hpp"

class Earth : public Planet {
public:
    Earth(const std::string& ModelPath, const std::string& dayTexturePath, const std::string& nightTexturePath, const std::string& cloudTexturePath, float scale, float orbitalRadius, float orbitalSpeed, float axialSpeed, float axialTiltAngle,
           bool hasGlow = false, float glowScale = 0.0f, glm::vec4 glowTint = glm::vec4(0.0f))
        : Planet(ModelPath, scale, orbitalRadius, orbitalSpeed, axialSpeed, axialTiltAngle, hasGlow, glowScale, glowTint) {
 
        // 1. Load your custom texture and get its ID
        unsigned int customDayTextureID = model.loadTexture(dayTexturePath);
        
        // You could load night/cloud textures here too if needed
        // unsigned int customNightTextureID = model.loadTexture(nightTexturePath);

        // 2. Find and replace the texture ID in the model's data structure
        // Loop through all the meshes that were loaded by Assimp
        for (Mesh& mesh : model.meshes) {
            // Loop through all the textures associated with THIS mesh
            for (Texture& texture : mesh.textures) {
                // Check if this is the diffuse texture (the main color texture)
                if (texture.type == "texture_diffuse") {
                    // We found it. Replace its ID with our new custom texture ID.
                    // It's a good idea to delete the old texture to prevent memory leaks.
                    glDeleteTextures(1, &texture.id); 
                    texture.id = customDayTextureID;
                    
                    // Optional: Update the path for debugging/consistency
                    texture.path = dayTexturePath;

                    // break from the inner loops if you only expect one diffuse texture per mesh
                }
            }
        }
    }
};

#endif  // INCLUDE_SOLAR_SYSTEM_EARTH_HPP_
