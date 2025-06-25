#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
// We no longer care about the model's built-in texture coordinates
// layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords; // We will generate this ourselves

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const float PI = 3.14159265359;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
 
    // --- Generate Spherical UVs from Vertex Position ---
    
    // 1. Normalize the position vector to ensure it's on a unit sphere.
    // This gives us a pure direction vector (x, y, z).
    vec3 pos_unit = normalize(aPos);

    // 2. Calculate theta (longitude) and phi (latitude).
    // atan(y, x) is equivalent to atan2(y, x) in GLSL
    float theta = atan(pos_unit.z, pos_unit.x); // Azimuthal angle
    float phi = asin(pos_unit.y);               // Polar angle

    // Way to map [-PI, PI] to [0, 1] and center it.
    float u = theta / (2.0 * PI) + 0.5;
    float v = 1.0 - ((phi + PI / 2.0) / PI); // Keeps the vertical flip

    // 4. Pass the newly calculated coordinates to the fragment shader.
    TexCoords = vec2(u, v);

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
