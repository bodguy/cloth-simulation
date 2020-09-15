#version 330 core

uniform vec4 aFragColor;
out vec4 FragColor;

void main() {
    FragColor = aFragColor;
}