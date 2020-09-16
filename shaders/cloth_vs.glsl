#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 WorldViewPos;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;

void main() {
    mat3 normalMatrix = transpose(inverse(mat3(model)));

    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.Normal = normalMatrix * aNormal;
    vs_out.WorldViewPos = vec3(model * vec4(viewPos, 1.0));

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}