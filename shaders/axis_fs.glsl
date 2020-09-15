#version 330 core

in vec3 aFragColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(aFragColor, 1.0);
}