#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D glowTexture;
uniform vec4 glowTint;

void main() {
    vec4 textureColor = texture(glowTexture, TexCoords);

    FragColor = textureColor * glowTint;
}
