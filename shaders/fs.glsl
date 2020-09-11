#version 330 core

#define NUM_POINT_LIGHTS 1

struct PointLight {
    vec3 position;
    vec3 color;
    float attenuation;
    float intensity;
};

struct Material {
    float shininess;
};

out vec4 FragColor;
in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 WorldViewPos;
} fs_in;

uniform PointLight pointLights[NUM_POINT_LIGHTS];
uniform Material material;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (1.0 + clamp(light.attenuation, 0.0, 1.0) * pow(distance, 2));

    // combine results
    vec3 ambient = light.color * light.intensity * attenuation;
    vec3 diffuse = ambient * diff; // intentional for the sake of performance
    vec3 specular = light.color * spec * attenuation;
    return (ambient + diffuse + specular);
}

void main() {
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(fs_in.WorldViewPos - fs_in.FragPos);
    vec3 result = vec3(0.0);
    for(int i = 0; i < NUM_POINT_LIGHTS; i++) {
        result += CalcPointLight(pointLights[i], normal, fs_in.FragPos, viewDir);
    }
    FragColor = vec4(result, 1.0);
}