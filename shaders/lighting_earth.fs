#version 330 core
out vec4 FragColor;

// Data from Vertex Shader
in vec3 FragPos;
in vec3 Normal; // We still use this for LIGHTING
in vec3 v_ModelSpacePos; // CHANGED: We now get position instead of normal

// Textures
uniform sampler2D texture_day;
uniform sampler2D texture_night;
uniform sampler2D texture_clouds;

// Uniforms
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float u_time;

const float PI = 3.14159265359;

void main()
{
    // 1. TEXTURE COORDINATE CALCULATION 
    vec3 p = normalize(v_ModelSpacePos);

    float u = (atan(p.x, p.z) / (2.0 * PI)) + 0.5;
    float v = 1.0 - ((asin(p.y) / PI) + 0.5);

    vec2 finalTexCoords = vec2(u, v);

    // 1.5. DERIVATIVE CALCULATION FOR SEAM FIX
    // Get the rate of change of the position vector for adjacent pixels
    vec3 dPdx = dFdx(p);
    vec3 dPdy = dFdy(p);


    // Use the chain rule to find the rate of change of the UV coordinates
    // This is the derivative of atan(p.x, p.z) and asin(p.y)
    float inv_denom_u = 1.0 / (p.x * p.x + p.z * p.z);
    float dUdx = (0.5 / PI) * (p.z * dPdx.x - p.x * dPdx.z) * inv_denom_u;
    float dUdy = (0.5 / PI) * (p.z * dPdy.x - p.x * dPdy.z) * inv_denom_u;

    // Handle potential division by zero at the poles for the v derivative
    float inv_denom_v = 1.0 / sqrt(max(0.0001, 1.0 - p.y * p.y));
    float dVdx = (1.0 / PI) * dPdx.y * inv_denom_v;
    float dVdy = (1.0 / PI) * dPdy.y * inv_denom_v;
    
    // Package the derivatives into vectors for textureGrad
    vec2 dUVdx = vec2(dUdx, dVdx);
    vec2 dUVdy = vec2(dUdy, dVdy);

    // Because we flipped the textures, the derivatives become negative
    dUVdx.y = dUVdx.y;
    dUVdy.y = dUVdy.y;

    // 2. LIGHTING CALCULATION
    vec3 lightColor = vec3(1.0, 1.0, 0.95);
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;
    float specularStrength = 0.2;
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;
    float rimPower = 2.0;
    float rim = pow(1.0 - abs(dot(norm, viewDir)), rimPower);
    vec3 rimColor = vec3(0.5, 0.7, 0.8) * rim;

    // 3. SAMPLE SURFACE AND CLOUDS
    vec3 dayColor = textureGrad(texture_day, finalTexCoords, dUVdx, dUVdy).rgb;
    vec3 nightColor = textureGrad(texture_night, finalTexCoords, dUVdx, dUVdy).rgb;

    float cloud_u_scrolling = fract(finalTexCoords.x + u_time * 0.03);
    vec2 cloudTexCoords = vec2(cloud_u_scrolling, v);
    float cloudOpacity = 0.6;
    float cloudIntensity = textureGrad(texture_clouds, cloudTexCoords, dUVdx, dUVdy).r;
    vec3 cloudColor = vec3(1.0);

    // 4. COMBINE EVERYTHING
    vec3 litSurface = ambient + diffuse;
    float nightSideFactor = 1.0 - smoothstep(-0.1, 0.2, diff);
    vec3 basePlanetColor = litSurface * (dayColor * (1.0 - nightSideFactor)) + (nightColor * nightSideFactor);

    vec3 colorWithClouds = mix(basePlanetColor, cloudColor, cloudIntensity * cloudOpacity * diff);

    vec3 finalColor = colorWithClouds + specular;
    finalColor += rimColor * (1.0 - diff) * 0.7;

    FragColor = vec4(finalColor, 1.0);
}

