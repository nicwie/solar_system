#ifndef INCLUDE_SOLAR_SYSTEM_EARTH_HPP_
#define INCLUDE_SOLAR_SYSTEM_EARTH_HPP_

#include <string>

#include "Planet.hpp"

class Earth : public Planet {
public:
    Earth(const std::string& ModelPath,
          const std::string& dayTexturePath,
          const std::string& nightTexturePath,
          const std::string& cloudTexturePath,
          const std::string& specularMapPath,
          float scale,
          float orbitalRadius,
          float orbitalSpeed,
          float axialSpeed,
          float axialTiltAngle,
          float ellipticity,
          bool hasGlow = false,
          float glowScale = 0.0f,
          glm::vec4 glowTint = glm::vec4(1.0f))
        : Planet(ModelPath, scale, orbitalRadius, orbitalSpeed, axialSpeed, axialTiltAngle, ellipticity, hasGlow, glowScale, glowTint)
    {
        m_model.addTexture(dayTexturePath, "texture_day");
        m_model.addTexture(nightTexturePath, "texture_night");
        m_model.addTexture(cloudTexturePath, "texture_clouds");
        m_model.addTexture(specularMapPath, "texture_specular");
    };
};

#endif  // INCLUDE_SOLAR_SYSTEM_EARTH_HPP_
