#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

// to avoid warning
uniform sampler2D texture_diffuse;
uniform sampler2D glowTexture;
uniform vec4 glowTint;

void main() {
    vec4 textureColor = texture(glowTexture, TexCoords);

    FragColor = textureColor * glowTint;
}
