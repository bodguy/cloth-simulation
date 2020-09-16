#include "utils.h"
#include <iostream>
#include <GL/glew.h>

bool read_file(const std::string& filepath, std::string& out_source) {
    FILE* fp = nullptr;
    fp = fopen(filepath.c_str(), "r");
    if (!fp) return false;
    fseek(fp, 0, SEEK_END);
    auto size = static_cast<size_t>(ftell(fp));
    fseek(fp, 0, SEEK_SET);
    char* buffer = (char*)malloc(sizeof(char) * size);
    if (!buffer) return false;
    fread(buffer, size, 1, fp);
    out_source.assign(buffer, size);
    free(buffer);
    fclose(fp);
    return true;
}

unsigned int load_shader_from_file(const std::string& vs_name, const std::string& fs_name) {
    std::string vertexSource, fragmentSource;
    bool result = read_file(vs_name, vertexSource);
    if (!result) return 0;
    result = read_file(fs_name, fragmentSource);
    if (!result) return 0;

    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderSource = vertexSource.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    // check for shader compile errors
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "shader compile error in " << vs_name << std::endl << infoLog << std::endl;
        return 0;
    }

    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentShaderSource = fragmentSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "shader compile error in " << fs_name << std::endl << infoLog << std::endl;
        return 0;
    }

    // link program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "program linking error with " << vs_name << " and " << fs_name << std::endl << infoLog << std::endl;
        return 0;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

unsigned int subdivide(unsigned int p1, unsigned int p2, std::vector<glm::vec3>& positions) {
    glm::vec3 middle = (positions[p1] + positions[p2]) / 2.f;
    positions.push_back(glm::normalize(middle));
    return positions.size() - 1;
}

std::vector<glm::vec3> generate_uv_sphere(float radius, int latitudes, int longitudes) {
    float latitude_increment = 360.0f / latitudes;
    float longitude_increment = 180.0f / longitudes;

    // if this causes an error, consider changing the size to [(latitude + 1)*(longitudes + 1)], but this should work.
    std::vector<glm::vec3> vertices{};

    for (float u = 0; u < 360.0f; u += latitude_increment) {
        for (float t = 0; t < 180.0f; t += longitude_increment) {
            float rad = radius;

            float x = (float) (rad * std::sin(glm::radians(t)) * std::sin(glm::radians(u)));
            float y = (float) (rad * std::cos(glm::radians(t)));
            float z = (float) (rad * std::sin(glm::radians(t)) * std::cos(glm::radians(u)));

            vertices.emplace_back(glm::vec3(x, y, z));

            float x1 = (float) (rad * std::sin(glm::radians(t + longitude_increment)) * std::sin(glm::radians(u + latitude_increment)));
            float y1 = (float) (rad * std::cos(glm::radians(t + longitude_increment)));
            float z1 = (float) (rad * std::sin(glm::radians(t + longitude_increment)) * std::cos(glm::radians(u + latitude_increment)));

            vertices.emplace_back(glm::vec3(x1, y1, z1));
        }
    }

    return vertices;

}

std::tuple<std::vector<glm::vec3>, std::vector<glm::vec3>, std::vector<unsigned int>> generate_ico_sphere(unsigned int subdivisions) {
    float t = (1.f + glm::sqrt(5.f) / 2.f);
    std::vector<glm::vec3> vertices = {
            glm::normalize(glm::vec3(-1.f, t, 0.f)), glm::normalize(glm::vec3(1.f, t, 0.f)),
            glm::normalize(glm::vec3(-1.f, -t, 0.f)), glm::normalize(glm::vec3(1.f, -t, 0.f)),
            glm::normalize(glm::vec3(0.f, -1.f, t)), glm::normalize(glm::vec3(0.f, 1.f, t)),
            glm::normalize(glm::vec3(0.f, -1.f, -t)), glm::normalize(glm::vec3(0.f, 1.f, -t)),
            glm::normalize(glm::vec3(t, 0.f, -1.f)), glm::normalize(glm::vec3(t, 0.f, 1.f)),
            glm::normalize(glm::vec3(-t, 0.f, -1.f)), glm::normalize(glm::vec3(-t, 0.f, 1.f))
    };

    std::vector<unsigned int> indices = {
            0, 11, 5, 0, 5,  1,  0,  1,  7,  0,  7, 10, 0, 10, 11,
            1, 5,  9, 5, 11, 4,  11, 10, 2,  10, 7, 6,  7, 1,  8,
            3, 9,  4, 3, 4,  2,  3,  2,  6,  3,  6, 8,  3, 8,  9,
            4, 9,  5, 2, 4,  11, 6,  2,  10, 8,  6, 7,  9, 8,  1
    };

    for (unsigned int i = 0; i < subdivisions; i++) {
        std::vector<unsigned int> indices2;
        for (unsigned int j = 0; j < indices.size() / 3; j++) {
            unsigned int a = subdivide(indices[j * 3 + 0], indices[j * 3 + 1], vertices);
            unsigned int b = subdivide(indices[j * 3 + 1], indices[j * 3 + 2], vertices);
            unsigned int c = subdivide(indices[j * 3 + 2], indices[j * 3 + 0], vertices);

            indices2.push_back(indices[j * 3 + 0]);
            indices2.push_back(a);
            indices2.push_back(c);

            indices2.push_back(indices[j * 3 + 1]);
            indices2.push_back(b);
            indices2.push_back(a);

            indices2.push_back(indices[j * 3 + 2]);
            indices2.push_back(c);
            indices2.push_back(b);

            indices2.push_back(a);
            indices2.push_back(b);
            indices2.push_back(c);
        }
        indices = indices2;
    }

    std::vector<glm::vec3> normals;
    normals.reserve(vertices.size());
    for (const auto& v : vertices) {
        normals.push_back(v);
    }

    return {vertices, normals, indices};
}

void error(const std::string& message) {
    std::cout << message << std::endl;
}