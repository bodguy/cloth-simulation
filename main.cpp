#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

std::vector<float> vertices = {
    -0.5f,  0.5f, 0.0f, //vertex 1 : Top-left
    0.5f, 0.5f, 0.0f, //vertex 2 : Top-right
    0.5f, -0.5f, 0.0f, //vertex 3 : Bottom-right
    0.5f, -0.5f, 0.0f, //vertex 4 : Bottom-right
    -0.5f, -0.5f, 0.0f, //vertex 5 : Bottom-left
    -0.5f,  0.5f, 0.0f //vertex 6 : Top-left
};

unsigned vao1, vbo1, program1;

bool loadFile(const std::string& filepath, std::string& out_source) {
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

unsigned int loadShaderFromFile(const std::string& vs_name, const std::string& fs_name) {
    std::string vertexSource, fragmentSource;
    bool result = loadFile(vs_name, vertexSource);
    if (!result) return 0;
    result = loadFile(fs_name, fragmentSource);
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

void error(const std::string& message) {
    std::cout << message << std::endl;
}

bool init() {
    glGenVertexArrays(1, &vao1);
    glGenBuffers(1, &vbo1);
    glBindVertexArray(vao1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo1);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices.front(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(NULL);

    program1 = loadShaderFromFile("../shaders/vs.cg", "../shaders/fs.cg");
    if (!program1)
        return false;

    return true;
}

void destroy() {
    glDeleteVertexArrays(1, &vao1);
    glDeleteBuffers(1, &vbo1);
    glDeleteProgram(program1);
}

void update(float dt) {

}

void render() {
    glm::mat4 model = glm::mat4(1.0f);
    glUseProgram(program1);
    glBindVertexArray(vao1);
    glUniformMatrix4fv(glGetUniformLocation(program1, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}

int main() {
    if (!glfwInit()) {
        error("glfw init error");
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    int w = 800, h = 600;
    std::string title = "test";
    GLFWwindow* window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
    if (!window) {
        error("glfw window init error");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwPollEvents();

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        error("glew init error");
        glfwTerminate();
        return -1;
    }

    if (!init()) {
        error("init failed");
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        update(1.f);

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
