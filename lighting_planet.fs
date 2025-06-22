#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform sampler2D texture_diffuse1;

void main()
{
    vec3 lightColor = vec3(1.0, 1.0, 0.95); 

    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 0.2;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 5.0); // last value is shininess
    vec3 specular = specularStrength * spec * lightColor;

    // Rim lighting
    float rimPower = 5; // sharpness of rim
    float rim = 1.0 - abs(dot(norm, viewDir));
    rim = pow(rim, rimPower);
    vec3 rimColor = vec3(0.5, 0.7, 0.8) * rim;

    vec3 objectColor = texture(texture_diffuse1, TexCoords).rgb;

    vec3 result = (ambient + diffuse) * objectColor + specular + (0.8 * rimColor);
    FragColor = vec4(result, 1.0);
}
