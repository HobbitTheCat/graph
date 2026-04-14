#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in int aColor;

uniform mat4 mvp;
out vec3 vColor;

vec3 hashColor(int id) {
    float r = fract(sin(float(id) * 12.9898) * 43758.5453);
    float g = fract(sin(float(id) * 78.233) * 43758.5453);
    float b = fract(sin(float(id) * 45.164) * 43758.5453);
    return vec3(r, g, b);
}

vec3 getPaletteColor(int id) {
    vec3 colors[5];
    colors[0] = vec3(0.0, 0.7, 1.0);
    colors[1] = vec3(1.0, 0.3, 0.3);
    colors[2] = vec3(0.3, 1.0, 0.3);
    colors[3] = vec3(1.0, 0.8, 0.2);
    colors[4] = vec3(0.8, 0.2, 1.0);
    return colors[id % 5];
}

void main() {
    gl_Position = mvp * vec4(aPos, 1.0);
    vColor = getPaletteColor(aColor);
}