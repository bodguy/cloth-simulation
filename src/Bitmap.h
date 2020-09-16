#ifndef CLOTH_SIMULATION_BITMAP_H
#define CLOTH_SIMULATION_BITMAP_H

#include <vector>
#include <cassert>

template<typename T>
class Bitmap {
public:
  Bitmap();
  Bitmap(int w, int h, T val);

  Bitmap<T>& operator=(const Bitmap<T>& other);

  void clear(int w, int h, T val);
  [[nodiscard]] int get_idx(int x, int y) const;
  T get(int x, int y) const;
  void set(int x, int y, T val);
  bool replace_part(Bitmap<T> const& other, int x_left, int y_bottom);

  int width{}, height{};
  std::vector<T> data;
};

template <typename T>
Bitmap<T>::Bitmap() :width(0), height(0), data() {

}

template <typename T>
Bitmap<T>::Bitmap(int w, int h, T val) : data() {
    clear(w, h, val);
}

template <typename T>
Bitmap<T>& Bitmap<T>::operator=(Bitmap<T> const& other) {
    replace_part(other, 0, 0);
    return *this;
}

template <typename T>
void Bitmap<T>::clear(int w, int h, T val) {
    data.resize(w*h);
    std::fill(data.begin(), data.end(), val);
    width = w;
    height = h;
}

template <typename T>
int Bitmap<T>::get_idx(const int x, const int y) const {
    if (x < 0 || y < 0 || x > width || y > height) {
        return -1;
    }
    int rows_from_bottom = (height - 1) - y;
    int arr_pos = width * rows_from_bottom;
    arr_pos += x;
    return arr_pos;
}

template <typename T>
T Bitmap<T>::get(const int x, const int y) const {
    int idx = get_idx(x, y);
    assert(idx >= 0);
    return data[idx];
}

template <typename T>
void Bitmap<T>::set(const int x, const int y, const T val) {
    int idx = get_idx(x, y);
    if (idx < 0) {
        return;
    }
    data[idx] = val;
}

template <typename T>
bool Bitmap<T>::replace_part(Bitmap<T> const& other, int x_left, int y_bottom) {
    if ((x_left + other.width) > width || (y_bottom + other.height) > height) {
        return false;
    }

    for (int row = 0; row < other.height; row ++) {
        for (int col = 0; col < other.width; col++) {
            set(col + x_left, row + y_bottom, other.get(col, row));
        }
    }
    return true;
}

#endif //CLOTH_SIMULATION_BITMAP_H
