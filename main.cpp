#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

class Particle {
public:
  Particle() = default;
  explicit Particle(const glm::vec3& position);

  glm::vec3 get_position() const { return m_position; }

  void offset_pos(const glm::vec3& v);
  void set_movable(bool movable);

  void add_force(const glm::vec3& force);
  void update(float dt);

private:
  glm::vec3 m_position{}, m_old_position{}, m_acceleration = glm::vec3(0, 0, 0);
  float m_mass = 1.f, m_damping = 0.01f;
  bool m_is_movable = true;
};

Particle::Particle(const glm::vec3 &position) : m_position{position}, m_old_position{position} {

}

void Particle::offset_pos(const glm::vec3 &v) {
    if (m_is_movable) {
        m_position += v;
    }
}

void Particle::set_movable(bool movable) {
    m_is_movable = movable;
}

void Particle::add_force(const glm::vec3& force) {
    m_acceleration += force / m_mass;
}

void Particle::update(float dt) {
    if (m_is_movable) {
        glm::vec3 old_position = m_position;
        m_position = m_position + (m_position - m_old_position) * (1.0f - m_damping) + m_acceleration * dt;
        m_old_position = old_position;
        m_acceleration = glm::vec3(0, 0, 0);
    }
}

class Constraint {
public:
  Constraint(Particle* p1, Particle* p2);
  void satisfy();

private:
  float m_rest_distance;
  Particle* m_p1, *m_p2;
};

Constraint::Constraint(Particle *p1, Particle *p2) : m_p1{p1}, m_p2{p2} {
    glm::vec3 v = m_p1->get_position() - m_p2->get_position();
    m_rest_distance = glm::length(v);
}

void Constraint::satisfy() {
    glm::vec3 p1_to_p2 = m_p2->get_position() - m_p1->get_position();
    float current_distance = glm::length(p1_to_p2);
    glm::vec3 correction_vec_half = p1_to_p2 * (1.f - m_rest_distance / current_distance) * 0.5f;
    m_p1->offset_pos(correction_vec_half);
    m_p2->offset_pos(-correction_vec_half);
}

class Cloth {
public:
  Cloth(int w, int h);
  ~Cloth();

  std::pair<std::vector<glm::vec3>, std::vector<glm::vec3>> make_data_buffer();
  void rebuild_vertex_buffer(bool first_invoked);
  void add_wind_force(const glm::vec3& direction);

  void render();
  void update(float dt);

  Particle* get_particle(int x, int y);

private:
  glm::vec3 calc_triangle_normal(Particle* p1, Particle* p2, Particle* p3) const;
  void add_wind_force_for_triangle(Particle* p1, Particle* p2, Particle* p3, const glm::vec3& direction);

private:
  int m_width, m_height;
  bool m_enabled = true, m_use_gravity = true;
  int m_cloth_solver_freq = 15;
  std::vector<Particle> m_particles;
  std::vector<Constraint> m_constraint;
  unsigned int vao = 0, vbo = 0, vbo2 = 0;
};

Cloth::Cloth(int w, int h) : m_width{w}, m_height{h} {
    m_particles.resize(m_width * m_height);

    // creating particles in a grid of particles from (0,0,0) to (width,-height,0)
    for (int x = 0; x < m_width; ++x) {
        for (int y = 0; y < m_height; ++y) {
            glm::vec3 position = glm::vec3(1.f * (x / (float)m_width), -1.f * (y / (float)m_height), 0.f);
            m_particles[y * m_width + x] = Particle(position);
        }
    }

    // Connecting immediate neighbor
    for (int x = 0; x < m_width; ++x) {
        for (int y = 0; y < m_height; ++y) {
            if (x < m_width - 1) m_constraint.emplace_back(Constraint(get_particle(x, y), get_particle(x + 1, y)));
            if (y < m_height - 1) m_constraint.emplace_back(Constraint(get_particle(x, y), get_particle(x, y + 1)));
            if (x < m_width - 1 && y < m_height - 1) {
                m_constraint.emplace_back(Constraint(get_particle(x, y), get_particle(x + 1, y + 1)));
                m_constraint.emplace_back(Constraint(get_particle(x + 1, y), get_particle(x, y + 1)));
            }
        }
    }

    // Connecting secondary neighbors
    for (int x = 0; x < m_width; ++x) {
        for (int y = 0; y < m_height; ++y) {
            if (x < m_width - 2) m_constraint.emplace_back(Constraint(get_particle(x, y), get_particle(x + 2, y)));
            if (y < m_height - 2) m_constraint.emplace_back(Constraint(get_particle(x, y), get_particle(x, y + 2)));
            if (x < m_width - 2 && y < m_height - 2) {
                m_constraint.emplace_back(Constraint(get_particle(x, y), get_particle(x + 2, y + 2)));
                m_constraint.emplace_back(Constraint(get_particle(x + 2, y), get_particle(x, y + 2)));
            }
        }
    }

    // disable top 3 particle to hold cloth
    for (int i = 0; i< 3; i++) {
        get_particle(0 + i, 0)->offset_pos(glm::vec3(0.5,0.0,0.0));
        get_particle(0 + i, 0)->set_movable(false);

        get_particle(0+i ,0)->offset_pos(glm::vec3(-0.5,0.0,0.0));
        get_particle(0 + m_width - 1 - i ,0)->set_movable(false);
    }
    rebuild_vertex_buffer(true);
}

Cloth::~Cloth() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &vbo2);
}

std::pair<std::vector<glm::vec3>, std::vector<glm::vec3>> Cloth::make_data_buffer() {
    std::vector<glm::vec3> vertex_position_buffer{};
    std::vector<glm::vec3> vertex_normal_buffer{};
    for (int x = 0; x < m_width - 1; x++) {
        for (int y = 0; y < m_height - 1; y++) {
            vertex_position_buffer.emplace_back(get_particle(x + 1, y)->get_position());
            vertex_position_buffer.emplace_back(get_particle(x, y)->get_position());
            vertex_position_buffer.emplace_back(get_particle(x, y + 1)->get_position());

            glm::vec3 normal = glm::normalize(calc_triangle_normal(get_particle(x + 1, y), get_particle(x, y), get_particle(x, y + 1)));
            vertex_normal_buffer.emplace_back(normal);
            vertex_normal_buffer.emplace_back(normal);
            vertex_normal_buffer.emplace_back(normal);

            vertex_position_buffer.emplace_back(get_particle(x + 1, y + 1)->get_position());
            vertex_position_buffer.emplace_back(get_particle(x + 1, y)->get_position());
            vertex_position_buffer.emplace_back(get_particle(x, y + 1)->get_position());
            normal = glm::normalize(calc_triangle_normal(get_particle(x + 1, y + 1), get_particle(x + 1, y), get_particle(x, y + 1)));
            vertex_normal_buffer.emplace_back(normal);
            vertex_normal_buffer.emplace_back(normal);
            vertex_normal_buffer.emplace_back(normal);
        }
    }
    return {vertex_position_buffer, vertex_normal_buffer};
}

void Cloth::rebuild_vertex_buffer(bool first_invoked) {
    auto buffer_data = make_data_buffer();

    if (first_invoked) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &vbo2);
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    if (first_invoked) {
        glBufferData(GL_ARRAY_BUFFER, buffer_data.first.size() * sizeof(glm::vec3), &(buffer_data.first[0].x), GL_DYNAMIC_DRAW);
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_data.first.size() * sizeof(glm::vec3), &(buffer_data.first[0].x));
    }
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    if (first_invoked) {
        glBufferData(GL_ARRAY_BUFFER, buffer_data.second.size() * sizeof(glm::vec3), &(buffer_data.second[0].x), GL_DYNAMIC_DRAW);
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_data.second.size() * sizeof(glm::vec3), &(buffer_data.second[0].x));
    }
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glBindVertexArray(NULL);

    if (!first_invoked) {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, buffer_data.first.size());
        glBindVertexArray(NULL);
    }
}

void Cloth::add_wind_force(const glm::vec3& direction) {
    for (int x = 0; x < m_width - 1; ++x) {
        for (int y = 0; y < m_height - 1; ++y) {
            add_wind_force_for_triangle(get_particle(x + 1, y), get_particle(x, y), get_particle(x, y + 1), direction);
            add_wind_force_for_triangle(get_particle(x + 1, y + 1), get_particle(x + 1, y), get_particle(x, y + 1), direction);
        }
    }
}

glm::vec3 Cloth::calc_triangle_normal(Particle *p1, Particle *p2, Particle *p3) const {
    glm::vec3 position1 = p1->get_position();
    glm::vec3 position2 = p2->get_position();
    glm::vec3 position3 = p3->get_position();

    glm::vec3 v1 = position2 - position1;
    glm::vec3 v2 = position3 - position1;

    return glm::cross(v1, v2);
}

void Cloth::add_wind_force_for_triangle(Particle* p1, Particle* p2, Particle* p3, const glm::vec3& direction) {
    glm::vec3 normal = calc_triangle_normal(p1, p2, p3);
    glm::vec3 d = glm::normalize(normal);
    glm::vec3 force = normal * glm::dot(d, direction);
    p1->add_force(force);
    p2->add_force(force);
    p3->add_force(force);
}

void Cloth::render() {
    rebuild_vertex_buffer(false);
}

void Cloth::update(float dt) {
    if (!m_enabled) return;

    for (int i = 0; i < m_cloth_solver_freq; i++) {
        for (auto& constraint : m_constraint) {
            constraint.satisfy();
        }
    }

    glm::vec3 gravity_dir = glm::vec3(0.f, -0.2f, 0.f);
    for (auto& particle : m_particles) {
        if (m_use_gravity) {
            particle.add_force(gravity_dir * dt);
        }
        particle.update(dt);
    }
}

Particle* Cloth::get_particle(int x, int y) {
    return &m_particles[y * m_width + x];
}

unsigned program1;
Cloth* cloth;
glm::vec3 dir = glm::vec3(0.5,0,0.2), pos = glm::vec3(0.27, -0.17, 2.04);
int w = 1024, h = 768;
GLFWwindow* window;
glm::vec3 forward = glm::vec3(0.f, 0.f, -1.f);
glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
glm::vec3 right = glm::cross(forward, up);
float nearClipPlane = 0.1f, farClipPlane = 100.f, fieldOfView = glm::radians(45.f), speed = 0.04f;
bool is_wireframe = true;

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
    program1 = loadShaderFromFile("../shaders/vs.cg", "../shaders/fs.cg");
    if (!program1)
        return false;

    cloth = new Cloth(55, 45);

    return true;
}

void destroy() {
    glDeleteProgram(program1);
    if (cloth) {
        delete cloth;
        cloth = nullptr;
    }
}

void fixedUpdate(float dt) {
    cloth->add_wind_force(dir);
    cloth->update(dt);
}

void update(float dt) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pos += forward * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pos += -forward * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) pos += -right * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) pos += right * speed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) pos += up * speed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) pos += -up * speed;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) is_wireframe = !is_wireframe;
    if (is_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void render() {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-0.5f, 0.5f, 0.f));
    glm::mat4 view = glm::lookAt(pos, pos + forward, up);
    glm::mat4 perspective = glm::perspective(fieldOfView, (float)w / (float)h, nearClipPlane, farClipPlane);

    glUseProgram(program1);
    glUniformMatrix4fv(glGetUniformLocation(program1, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(program1, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(program1, "projection"), 1, GL_FALSE, glm::value_ptr(perspective));
    cloth->render();
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

    std::string title = "test";
    window = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
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

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        float fixed_timestamp = 0.25f;
        update(0.f);
        fixedUpdate(fixed_timestamp);

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    destroy();
    glfwTerminate();

    return 0;
}
