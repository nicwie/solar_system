#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

// Pass-throughs to the fragment shader
out vec3 FragPos;
out vec3 Normal;
out vec3 v_ModelSpacePos; // ADDED: Pass the original position

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;



    v_ModelSpacePos = aPos;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
