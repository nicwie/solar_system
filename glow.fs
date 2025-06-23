#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D glowTexture;

void main() {
    FragColor = 0.5 * texture(glowTexture, TexCoords);
}
