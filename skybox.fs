#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183); // 1/2PI, 1/PI

// Convert 3D lookup direction into 2D coordinate
vec2 SampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {
    vec2 uv = SampleSphericalMap(normalize(TexCoords));
    FragColor = texture(equirectangularMap, uv);
}
