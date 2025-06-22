#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

// Define a struct to hold all material properties
struct Material {
    sampler2D texture_diffuse1;
};

uniform Material material;

void main() {
    // Access the texture through the material struct
    FragColor = texture(material.texture_diffuse1, TexCoords);
}
