#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 v_ModelSpacePos;

uniform float u_time;
uniform vec3 viewPos;

const float PI = 3.14159265359;

// A PRNG for 3D coordinates.
float random(vec3 p) {
    return fract(sin(dot(p, vec3(12.9898, 78.233, 151.7182))) * 43758.5453123);
}

/**
 * @brief A 3D Value Noise function.
 */
float value_noise_3D(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);

    // Smoothstep curve for interpolation
    vec3 u = f * f * (3.0 - 2.0 * f);

    // Get random values for the 8 corners of the cube
    float c000 = random(i + vec3(0.0, 0.0, 0.0));
    float c100 = random(i + vec3(1.0, 0.0, 0.0));
    float c010 = random(i + vec3(0.0, 1.0, 0.0));
    float c110 = random(i + vec3(1.0, 1.0, 0.0));
    float c001 = random(i + vec3(0.0, 0.0, 1.0));
    float c101 = random(i + vec3(1.0, 0.0, 1.0));
    float c011 = random(i + vec3(0.0, 1.0, 1.0));
    float c111 = random(i + vec3(1.0, 1.0, 1.0));

    // Interpolate along x-axis
    float r0 = mix(c000, c100, u.x);
    float r1 = mix(c010, c110, u.x);
    float r2 = mix(c001, c101, u.x);
    float r3 = mix(c011, c111, u.x);

    // Interpolate along y-axis
    float ry0 = mix(r0, r1, u.y);
    float ry1 = mix(r2, r3, u.y);

    // Interpolate along z-axis
    return mix(ry0, ry1, u.z);
}

/**
 * @brief A 3D Fractional Brownian Motion (fbm) function.
 * Creates fractal noise by layering multiple octaves of 3D value noise.
 */
float fbm_3D(vec3 p) {
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 6; i++) {
        value += amplitude * value_noise_3D(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

void main() {
    vec3 perfectSphereVec = normalize(v_ModelSpacePos);

    // Procedural Noise
    vec3 p = perfectSphereVec * 2.0;
    float noise = fbm_3D(p + u_time * 0.05) * 0.5 + fbm_3D(p - u_time * 0.02) * 0.5;

    // Color Mapping
    vec3 color1 = vec3(1.0, 0.3, 0.0); // Dark orange
    vec3 color2 = vec3(1.0, 0.8, 0.0); // Bright yellow/white
    vec3 surfaceColor = mix(color1, color2, noise);

    // Limb Darkening and Rim Effect
    vec3 viewDir = normalize(viewPos - FragPos);
    // Use smoothstep to create a soft edge instead of a sharp falloff
    float limb = smoothstep(0.0, 0.6, dot(perfectSphereVec, viewDir));

    // Define a dark, reddish color for the edge of the sun
    vec3 rimColor = vec3(0.6, 0.1, 0.0);

    // Blend the surface color with the rim color based on the limb factor.
    vec3 finalColor = mix(rimColor, surfaceColor, limb);

    // Add a very bright core based on the noise, also faded by the limb factor.
    finalColor += vec3(1.0, 1.0, 0.5) * pow(noise, 3.0) * limb;

    FragColor = vec4(finalColor, 1.0);
}
