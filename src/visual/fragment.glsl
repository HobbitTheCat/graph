#version 330 core
out vec4 FragColor;
in vec3 vColor;
uniform vec3 color;
uniform bool useVertexColor;

void main() {
    if (useVertexColor) {
        FragColor = vec4(vColor, 1.0);
    } else {
        FragColor = vec4(color, 1.0);
    }
}