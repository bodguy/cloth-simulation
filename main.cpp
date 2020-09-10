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
  glm::vec3 get_normal() const { return m_normal; }

  void offset_pos(const glm::vec3& v);
  void offset_normal(const glm::vec3& n) { m_normal += glm::normalize(n); }
  void set_movable(bool movable);
  void reset_normal() { m_normal = glm::vec3(0, 0, 0); }

  void add_force(const glm::vec3& force);
  void update(float dt);

private:
  glm::vec3 m_position{}, m_old_position{}, m_acceleration = glm::vec3(0, 0, 0), m_normal = glm::vec3(0, 0, 0);
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

  std::vector<glm::vec3> make_data_buffer();
  void initVertex();
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
  unsigned int vao = 0, vbo = 0;
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
    initVertex();
}

Cloth::~Cloth() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

std::vector<glm::vec3> Cloth::make_data_buffer() {
    std::vector<glm::vec3> data_buffer{};
    for (int x = 0; x < m_width - 1; x++) {
        for (int y = 0; y < m_height - 1; y++) {
            glm::vec3 normal = calc_triangle_normal(get_particle(x + 1, y), get_particle(x, y), get_particle(x, y + 1));
            data_buffer.emplace_back(get_particle(x + 1, y)->get_position());
            data_buffer.emplace_back(glm::normalize(normal));
            data_buffer.emplace_back(get_particle(x, y)->get_position());
            data_buffer.emplace_back(glm::normalize(normal));
            data_buffer.emplace_back(get_particle(x, y + 1)->get_position());
            data_buffer.emplace_back(glm::normalize(normal));

            normal = calc_triangle_normal(get_particle(x + 1, y + 1), get_particle(x + 1, y), get_particle(x, y + 1));
            data_buffer.emplace_back(get_particle(x + 1, y + 1)->get_position());
            data_buffer.emplace_back(glm::normalize(normal));
            data_buffer.emplace_back(get_particle(x + 1, y)->get_position());
            data_buffer.emplace_back(glm::normalize(normal));
            data_buffer.emplace_back(get_particle(x, y + 1)->get_position());
            data_buffer.emplace_back(glm::normalize(normal));
        }
    }
    return data_buffer;
}

void Cloth::initVertex() {
    std::vector<glm::vec3> data_buffer = make_data_buffer();
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    int STRIDE = 6 * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, data_buffer.size() * STRIDE, &data_buffer[0], GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void *)(3 * sizeof(float)));
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
    std::vector<glm::vec3> data_buffer = make_data_buffer();
    int STRIDE = 6 * sizeof(float);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, data_buffer.size() * STRIDE, &data_buffer[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void *)(3 * sizeof(float)));
    glDrawArrays(GL_TRIANGLES, 0, data_buffer.size());
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

std::vector<float> vertices = {
    -0.5f,  0.5f, 0.0f, //vertex 1 : Top-left
    0.5f, 0.5f, 0.0f, //vertex 2 : Top-right
    0.5f, -0.5f, 0.0f, //vertex 3 : Bottom-right
    0.5f, -0.5f, 0.0f, //vertex 4 : Bottom-right
    -0.5f, -0.5f, 0.0f, //vertex 5 : Bottom-left
    -0.5f,  0.5f, 0.0f //vertex 6 : Top-left
};

unsigned vao1, vbo1, program1;
Cloth* cloth;
glm::vec3 dir = glm::vec3(0.5,0,0.2);

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

    cloth = new Cloth(55,45);

    return true;
}

void destroy() {
    glDeleteVertexArrays(1, &vao1);
    glDeleteBuffers(1, &vbo1);
    glDeleteProgram(program1);
    if (cloth) {
        delete cloth;
        cloth = nullptr;
    }
}

void update(float dt) {
    cloth->add_wind_force(dir);
    cloth->update(dt);
}

void render() {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-0.5f, 0.5f, 0.f));

    glUseProgram(program1);
//    glBindVertexArray(vao1);
    glUniformMatrix4fv(glGetUniformLocation(program1, "model"), 1, GL_FALSE, glm::value_ptr(model));
//    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
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

    int w = 1024, h = 768;
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

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        float fixed_timestamp = 0.25f;
        update(fixed_timestamp);

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
