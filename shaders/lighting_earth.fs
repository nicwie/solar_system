// #version 330 core
// out vec4 FragColor;
//
// in vec3 FragPos;
// in vec3 Normal;
// in vec2 TexCoords;
//
// uniform vec3 lightPos;
// uniform vec3 viewPos;
//
// uniform sampler2D texture_day;
// uniform sampler2D texture_night;
//
//
// void main()
// {
//     // setup
//     vec3 lightColor = vec3(1.0, 1.0, 0.95); 
//     vec3 norm = normalize(Normal);
//     vec3 lightDir = normalize(lightPos - FragPos);
//     float diff = max(dot(norm, lightDir), 0.0);
//
//     // Texture Sampling
//     vec3 dayColor = texture(texture_day, TexCoords).rgb;
//     vec3 nightColor = texture(texture_night, TexCoords).rgb;
//
//     // Ambient
//     float ambientStrength = 0.3;
//     vec3 ambient = ambientStrength * lightColor;
//
//     // Diffuse
//     vec3 diffuse = diff * lightColor;
//
//     // Specular
//     float specularStrength = 0.2;
//     vec3 viewDir = normalize(viewPos - FragPos);
//     vec3 halfwayDir = normalize(lightDir + viewDir);
//     float spec = pow(max(dot(norm, halfwayDir), 0.0), 5.0); // last value is shininess
//     vec3 specular = specularStrength * spec * lightColor;
//
//     // Rim lighting
//     float rimPower = 2; // sharpness of rim
//     float rim = 1.0 - abs(dot(norm, viewDir));
//     rim = pow(rim, rimPower);
//     vec3 rimColor = vec3(0.5, 0.7, 0.8) * rim;
//
//     // "Darkness" to not light front with rimlighting
//     float darkness = 1.0 - diff;
//
//
//
//     // Night side
//     float nightSideFactor = 1.0 - smoothstep(-0.1, 0.2, diff);
//
//
//     vec3 surfaceColor = ambient + diffuse;
//
//     surfaceColor += specular;
//
//     vec3 finalColor = surfaceColor * (dayColor * (1 - nightSideFactor)) + (nightColor * nightSideFactor) + (rim * 0.1);
//
//     FragColor = vec4(finalColor, 1.0);
// }


#version 330 core
out vec4 FragColor;

// We only get the texture coordinates from the vertex shader
in vec2 TexCoords;

// We only need one texture for this test
uniform sampler2D texture_diffuse1;

void main()
{
    // Ignore all lighting, blending, and effects.
    // Just output the color directly from the day texture.
    FragColor = texture(texture_diffuse1, TexCoords);
    // FragColor = vec4(TexCoords.x, TexCoords.y, 0.0, 1.0);
}
