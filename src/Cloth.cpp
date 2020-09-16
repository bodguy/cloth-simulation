#include "Cloth.h"
#include <GL/glew.h>

glm::vec3 Cloth::gravity_dir = glm::vec3(0.f, -0.2f, 0.f);

Particle::Particle(const glm::vec3 &position) : m_position{position}, m_old_position{position} {

}

glm::vec3 Particle::get_position() const { return m_position; }
glm::vec3& Particle::get_normal() { return m_accumulated_normal; }
void Particle::add_to_normal(glm::vec3& normal) { m_accumulated_normal += normal; }
void Particle::reset_normal() { m_accumulated_normal = glm::vec3(0, 0, 0); }

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

std::tuple<std::vector<glm::vec3>, std::vector<glm::vec3>, std::vector<glm::vec2>> Cloth::make_data_buffer() {
    std::vector<glm::vec3> vertex_position_buffer{};
    vertex_position_buffer.reserve(6 * m_width * m_height);
    std::vector<glm::vec3> vertex_normal_buffer{};
    vertex_normal_buffer.reserve(6 * m_width * m_height);
    std::vector<glm::vec2> vertex_tex_buffer{};
    for(Particle& p : m_particles) {
        p.reset_normal();
    }

    for (int x = 0; x < m_width - 1; x++) {
        for (int y = 0; y < m_height - 1; y++) {
            vertex_position_buffer.emplace_back(get_particle(x + 1, y)->get_position());
            vertex_position_buffer.emplace_back(get_particle(x, y)->get_position());
            vertex_position_buffer.emplace_back(get_particle(x, y + 1)->get_position());
            glm::vec3 normal = glm::normalize(calc_triangle_normal(get_particle(x + 1, y), get_particle(x, y), get_particle(x, y + 1)));
            get_particle(x + 1, y)->add_to_normal(normal);
            get_particle(x, y)->add_to_normal(normal);
            get_particle(x, y + 1)->add_to_normal(normal);
            vertex_normal_buffer.emplace_back(get_particle(x + 1, y)->get_normal());
            vertex_normal_buffer.emplace_back(get_particle(x, y)->get_normal());
            vertex_normal_buffer.emplace_back(get_particle(x, y + 1)->get_normal());
//            vertex_tex_buffer.emplace_back(glm::font_vec2((float)x/m_width, (float)y/m_height));

            vertex_position_buffer.emplace_back(get_particle(x + 1, y + 1)->get_position());
            vertex_position_buffer.emplace_back(get_particle(x + 1, y)->get_position());
            vertex_position_buffer.emplace_back(get_particle(x, y + 1)->get_position());
            normal = glm::normalize(calc_triangle_normal(get_particle(x + 1, y + 1), get_particle(x + 1, y), get_particle(x, y + 1)));
            get_particle(x + 1, y + 1)->add_to_normal(normal);
            get_particle(x + 1, y)->add_to_normal(normal);
            get_particle(x, y + 1)->add_to_normal(normal);
            vertex_normal_buffer.emplace_back(get_particle(x + 1, y + 1)->get_normal());
            vertex_normal_buffer.emplace_back(get_particle(x + 1, y)->get_normal());
            vertex_normal_buffer.emplace_back(get_particle(x, y + 1)->get_normal());
        }
    }

    return {vertex_position_buffer, vertex_normal_buffer, vertex_tex_buffer};
}

void Cloth::rebuild_vertex_buffer(bool first_invoked) {
    const auto& [positions, normals, texcoords] = make_data_buffer();

    if (first_invoked) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &vbo2);
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    if (first_invoked) {
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), &(positions.front()), GL_DYNAMIC_DRAW);
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), &(positions.front()));
    }
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    if (first_invoked) {
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &(normals.front()), GL_DYNAMIC_DRAW);
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, normals.size() * sizeof(glm::vec3), &(normals.front()));
    }
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glBindVertexArray(NULL);

    if (!first_invoked) {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, positions.size());
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

    for (int i = 0; i < constraint_iterations; i++) {
        for (auto& constraint : m_constraint) {
            constraint.satisfy();
        }
    }

    for (auto& p : m_particles) {
        if (m_use_gravity) {
            p.add_force(gravity_dir * dt);
        }
        p.update(dt);
    }
}

void Cloth::collision_detection_with_sphere(const glm::vec3& center, const float radius) {
    for (auto& p : m_particles) {
        glm::vec3 distance = p.get_position() - center;
        float length = glm::length(distance);
        if (length < radius) {
            p.offset_pos(glm::normalize(distance) * (radius - length));
        }
    }
}

Particle* Cloth::get_particle(int x, int y) {
    return &m_particles[y * m_width + x];
}