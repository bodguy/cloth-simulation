#ifndef CLOTH_SIMULATION_APPLICATION_H
#define CLOTH_SIMULATION_APPLICATION_H

#include <glm/gtc/type_ptr.hpp>
#include <string>

class GLFWwindow;
class Cloth;
class Application {
public:
  Application(std::string title, int w, int h);
  ~Application();

  bool initApp();
  int loop();

private:
  bool init();
  void fixedUpdate(float dt);
  void update(float dt);
  void render();

private:
  int window_width, window_height;
  std::string app_title;
  unsigned cloth_shader{}, axis_shader{}, grid_shader{};
  unsigned sphere_vao{}, sphere_vbo_position{}, sphere_vbo_normal{}, sphere_ibo{};
  unsigned axis_line_vao{}, axis_line_vbo{};
  unsigned sphere_vao2{}, sphere_vbo_position2{};
  unsigned grid_vao{}, grid_vbo{}, grid_ibo{};
  glm::vec4 grid_color = glm::vec4(0, 1, 1, 1);
  glm::vec3 wind_dir = glm::vec3(12, 0, 0.6), viewPos = glm::vec3(0.27, -0.17, 2.04);
  glm::vec3 forward = glm::vec3(0.f, 0.f, -1.f), up = glm::vec3(0.f, 1.f, 0.f), right = glm::cross(forward, up);
  float nearClipPlane = 0.1f, farClipPlane = 100.f, fieldOfView = glm::radians(45.f), speed = 0.04f;
  bool is_wireframe = true;
  unsigned grid_draw_call_count = 0, sphere_draw_call_count = 0, sphere_draw_call_count2 = 0;
  glm::vec3 sphere_pos = glm::vec3(0, 0, 0);
  float sphere_radius = 0.2;
  GLFWwindow* window{};
  Cloth* cloth{};
};

#endif //CLOTH_SIMULATION_APPLICATION_H
