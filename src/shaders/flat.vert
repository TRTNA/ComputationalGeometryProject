#version 410 core


layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;

// model matrix
uniform mat4 modelMatrix;
// view matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;

out vec4 inColor;

void main() {
    inColor = color;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}