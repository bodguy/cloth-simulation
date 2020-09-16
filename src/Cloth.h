#ifndef CLOTH_SIMULATION_CLOTH_H
#define CLOTH_SIMULATION_CLOTH_H

#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <tuple>

class Particle {
public:
  Particle() = default;
  explicit Particle(const glm::vec3& position);

  glm::vec3 get_position() const;
  glm::vec3& get_normal();
  void add_to_normal(glm::vec3& normal);
  void reset_normal();

  void offset_pos(const glm::vec3& v);
  void set_movable(bool movable);

  void add_force(const glm::vec3& force);
  void update(float dt);

private:
  glm::vec3 m_position{}, m_old_position{}, m_acceleration = glm::vec3(0, 0, 0), m_accumulated_normal = glm::vec3(0, 0, 0);
  float m_mass = 1.f, m_damping = 0.01f;
  bool m_is_movable = true;
};

class Constraint {
public:
  Constraint(Particle* p1, Particle* p2);
  void satisfy();

private:
  float m_rest_distance;
  Particle* m_p1, *m_p2;
};

class Cloth {
public:
  Cloth(int w, int h);
  ~Cloth();

  std::tuple<std::vector<glm::vec3>, std::vector<glm::vec3>, std::vector<glm::vec2>> make_data_buffer();
  void rebuild_vertex_buffer(bool first_invoked);
  void add_wind_force(const glm::vec3& direction);

  void render();
  void update(float dt);
  void collision_detection_with_sphere(const glm::vec3& center, float radius);

  Particle* get_particle(int x, int y);

private:
  glm::vec3 calc_triangle_normal(Particle* p1, Particle* p2, Particle* p3) const;
  void add_wind_force_for_triangle(Particle* p1, Particle* p2, Particle* p3, const glm::vec3& direction);

private:
  int m_width, m_height;
  bool m_enabled = true, m_use_gravity = true;
  int constraint_iterations = 15;
  std::vector<Particle> m_particles;
  std::vector<Constraint> m_constraint;
  unsigned int vao = 0, vbo = 0, vbo2 = 0;
  static glm::vec3 gravity_dir;
};

#endif //CLOTH_SIMULATION_CLOTH_H
