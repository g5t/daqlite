#pragma once

class bifrostHistograms {
public:
  bifrostHistograms() {
    reset();
  };
  ~bifrostHistograms() = default;


  inline static int arc(int group){
    return group / 3;
  }

  inline static int module(int fiber){
    return fiber / 2;
  }

  inline static int triplet(int fiber, int group){
    int type = group % 3;
    return module(fiber) * 3 + type;
  }

  bool add(int fiber, int group, int a, int b){
    auto arc_ = arc(group);
    auto triplet_ = triplet(fiber, group);
    if (arc_ < 0 || arc_ >= ARCS || triplet_ < 0 || triplet_ >= TRIPLETS) {
      return false;
    }
    bool ok{true};
    ok &= add_ab(arc_, triplet_, a, b);
    ok &= add_phs(arc_, triplet_, a, b);
    ok &= add_pos(arc_, triplet_, a, b);
    return ok;
  }

  static constexpr int ARCS = 5;
  static constexpr int TRIPLETS = 9;
  static constexpr int RANGE = 1 << 15;
  static constexpr int SHIFT2D = 6;
  static constexpr int SHIFT1D = 5;
  static constexpr int BIN2D = RANGE >> SHIFT2D;
  static constexpr int BIN1D = RANGE >> SHIFT1D;

  int ab_data[ARCS][TRIPLETS][BIN2D][BIN2D]{};
  int phs_data[ARCS][TRIPLETS][BIN1D]{};
  int pos_data[ARCS][TRIPLETS][BIN1D]{};

  void reset() {
    memset(&ab_data, 0, sizeof(ab_data));
    memset(&phs_data, 0, sizeof(phs_data));
    memset(&pos_data, 0, sizeof(pos_data));
  }

  [[nodiscard]] bool is_empty(int arc, int triplet) const {
    for (int i = 0; i < BIN1D; ++i){
      if (phs_data[arc][triplet][i]){
        return false;
      }
    }
    return true;
  }

private:
  bool add_ab(int arc, int triplet, int a, int b){
    // A and B are _positive_ 16-bit signed integers (or there's a problem)
    // We need to convert them to 512 bins, so we shift them by 6 bits
    int x = a >> SHIFT2D;
    int y = b >> SHIFT2D;
    if (x < 0 || x >= BIN2D || y < 0 || y >= BIN2D) {
      return false;
    }
    ++ab_data[arc][triplet][x][y];
    return true;
  }
  bool add_phs(int arc, int triplet, int a, int b){
    if (a < 0 || b < 0){
      return false;
    }
    // right-shift by 5 to convert from 2^15 values to 1024 bins
    int phs = (a + b) >> SHIFT1D;
    if (phs >= BIN1D) {
      return false;
    }
    ++phs_data[arc][triplet][phs];
    return true;
  }
  bool add_pos(int arc, int triplet, int a, int b){
    if (a < 0 || b < 0){
      return false;
    }
    int num = a - b;
    int den = a + b;
    if (den == 0){
      return false;
    }
    double ratio = static_cast<double>(num) / static_cast<double>(den);
    if (ratio < -1.0 || ratio > 1.0){
      return false;
    }
    // full range is (-1, 1) so shift up by 1, multiply by 512 and convert to an integer
    auto x = static_cast<int>((ratio + 1.0) / 2.0 * (BIN1D - 1));

    if (x < 0 || x >= BIN1D) {
      return false;
    }
    ++pos_data[arc][triplet][x];
    return true;
  }

};