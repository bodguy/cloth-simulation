#ifndef CLOTH_SIMULATION_UTILS_H
#define CLOTH_SIMULATION_UTILS_H

#include <string>
#include <vector>
#include <map>
#include <glm/gtc/type_ptr.hpp>
#include "Bitmap.h"

bool read_file(const std::string& filepath, std::string& out_source);
unsigned int load_shader_from_file(const std::string& vs_name, const std::string& fs_name);
unsigned int subdivide(unsigned int p1, unsigned int p2, std::vector<glm::vec3>& positions);
std::vector<glm::vec3> generate_uv_sphere(float radius, int latitudes, int longitudes);
std::tuple<std::vector<glm::vec3>, std::vector<glm::vec3>, std::vector<unsigned int>> generate_ico_sphere(unsigned int subdivisions);
void error(const std::string& message);

template<typename T>
struct font_vec2 {
  T x, y;
};

struct glyph_info {
  glyph_info() : bitmap() { }
  font_vec2<int> size, bearing;
  int advance, ascender, descender, line_gap;
  Bitmap<unsigned char> bitmap;
  std::map<uint32_t, int> kerning;
};

struct font_info {
  int pixel_height;
  int ascender, descender;
  int line_gap;
};

#endif //CLOTH_SIMULATION_UTILS_H
