#include <iostream>
#include "data_manager.h"


int bifrost::data::hist_a_or_b(int x, int shift, int bins){
    int y = x >> shift;
    if (y < 0 || y >= bins) y = -1;
    return y;
}
int bifrost::data::hist_p(int x, int shift, int bins){
    // a and b are effectively 15-bit integers
    // so a+b is 16-bits, but we want this to fit into BIN2D bins,
    // so we must shift by an extra bit compared to a or b above
    int y = x >> (shift + 1);
    if (y < 0 || y >= bins) y = -1;
    return y;
}
int bifrost::data::hist_t(double x, int bins){
    // time at ESS resets every 1/14 Hz ~= 70 msec.
    // find the modulus and bin that range
    double period = 1.0 / 14.0;
    auto frac = fmod(x, period) / period;
    auto y = static_cast<int>(frac * bins);
    if (y < 0 || y >= bins) y = -1;
    return y;
}
int bifrost::data::hist_x(int a, int b, int bins){
    int num = a - b;
    int den = a + b;
    if (den == 0) {
        return false;
    }
    double ratio = static_cast<double>(num) / static_cast<double>(den);
    if (ratio < -1.0 || ratio > 1.0) {
        return false;
    }
    // full range is (-1, 1) so shift up by 1, multiply by 512 and convert to an integer
    auto x = static_cast<int>((ratio + 1.0) / 2.0 * (bins - 1));
    if (x < 0 || x >= bins) x = -1;
    return x;
}


bool bifrost::data::Manager::add(int fiber, int group, int a, int b, double time){
    auto arc_ = arc(group);
    auto triplet_ = triplet(fiber, group);
    if (arc_ < 0 || arc_ >= arcs || triplet_ < 0 || triplet_ >= triplets) {
        return false;
    }
    bool ok{true};
    ok &= add_1D(arc_, triplet_, a, b, time);
    ok &= add_2D(arc_, triplet_, a, b, time);
    return ok;
}

bool bifrost::data::Manager::add_1D(int arc, int triplet, int a, int b, double time){
    auto t_a = std::make_pair(Type::a, hist_a_or_b(a, SHIFT1D, BIN1D));
    auto t_b = std::make_pair(Type::b, hist_a_or_b(b, SHIFT1D, BIN1D));
    auto t_p = std::make_pair(Type::p, hist_p(a+b, SHIFT1D, BIN1D));
    auto t_x = std::make_pair(Type::x, hist_x(a, b, BIN1D));
    auto t_t = std::make_pair(Type::t, hist_t(time, BIN1D));
    if (t_a.second < 0 || t_b.second < 0 || t_p.second < 0 || t_x.second < 0 || t_t.second < 0) {
        return false;
    }
    for (auto [t, h]: {t_a, t_b, t_p, t_x, t_t}) {
      auto key = std::make_tuple(arc, triplet, t);
      if (!data_1d.count(key)) {
        std::cout << "arc" << arc << "triplet" << triplet << "does not exist in data_1d" << std::endl;
        return false;
      }
      if (static_cast<size_t>(h) < data_1d.at(key)->size()){
        data_1d.at(key)->at(h) += 1;
      } else {
        std::cout << "arc " << arc << " triplet " << triplet << " a " << a << " b " << b << " time " << time << "gives an out of bounds index " << h << std::endl;
      }
    }
    return true;
}

bool bifrost::data::Manager::add_2D(int arc, int triplet, int full_a, int full_b, double full_t){
    auto a = hist_a_or_b(full_a, SHIFT2D, BIN2D);
    auto b = hist_a_or_b(full_b, SHIFT2D, BIN2D);
    auto p = hist_p(full_a+full_b, SHIFT2D, BIN2D);
    auto x = hist_x(full_a, full_b, BIN2D);
    auto t = hist_t(full_t, BIN2D);
    if (a < 0 || b < 0 || p < 0 || x < 0 || t < 0) return false;
    std::vector<std::tuple<Type, int, int>> t_i_j {{Type::ab, a, b}, {Type::pt, p, t}, {Type::xt, x, t}, {Type::xp, x, p}};
    for (auto [y, i, j]: t_i_j){
        auto key = std::make_tuple(arc, triplet, y);
        if (!data_2d.count(key)) {
          std::cout << "arc" << arc << "triplet" << triplet << "does not exist in data_2d" << std::endl;
          return false;
        }
        if (i < data_2d[key]->keySize() && j < data_2d[key]->valueSize()){
          data_2d[key]->setCell(i, j, 1+data_2d[key]->cell(i, j));
        } else {
          std::cout << "arc " << arc << " triplet " << triplet << " a " << full_a << " b " << full_b << " time " << full_t << "gives an out of bounds index " << i << " " << j << std::endl;
        }
    }
    return true;
}

double bifrost::data::Manager::max() const {
    double x{-1};
    for (int a=0; a<arcs; ++a){
        auto y = max(a);
        if (y > x) x = y;
    }
    return x;
}

double bifrost::data::Manager::max(int arc) const {
    double x{-1};
    for (int t=0; t<triplets; ++t){
        auto y = max(arc, t);
        if (y > x) x = y;
    }
    return x;
}

double bifrost::data::Manager::max(int arc, int triplet) const {
    double x{-1};
    if (arc < 0 || arc >= arcs || triplet < 0 || triplet >= triplets) return x;
    for (auto t: TYPE1D){
        auto y = max_1D(arc, triplet, t);
        if (y > x) x = y;
    }
    for (auto t: TYPE2D){
        auto y = max_2D(arc, triplet, t);
        if (y > x) x = y;
    }
    return x;
}

double bifrost::data::Manager::max(int arc, int triplet, Type type) const {
    double x{-1};
    if (Type::a == type || Type::b == type || Type::p == type || Type::x == type || Type::t == type){
        auto y = max_1D(arc, triplet, type);
        if (y > x) x = y;
    }
    if (Type::ab == type || Type::xp == type || Type::xt == type || Type::pt == type){
        auto y = max_2D(arc, triplet, type);
        if (y > x) x = y;
    }
    return x;
}

double bifrost::data::Manager::max(int arc, Type type) const {
    double x{-1};
    if (Type::a == type || Type::b == type || Type::p == type || Type::x == type || Type::t == type){
        for (int t=0; t<triplets; ++t){
            auto y = max_1D(arc, t, type);
            if (y > x) x = y;
        }
    }
    if (Type::ab == type || Type::xp == type || Type::xt == type || Type::pt == type){
        for (int t=0; t<triplets; ++t){
            auto y = max_2D(arc, t, type);
            if (y > x) x = y;
        }
    }
    return x;
}



double bifrost::data::Manager::min() const {
  double x{max()};
  for (int a=0; a<arcs; ++a){
    auto y = min(a);
    if (y < x) x = y;
  }
  return x;
}

double bifrost::data::Manager::min(int arc) const {
  double x{max(arc)};
  for (int t=0; t<triplets; ++t){
    auto y = min(arc, t);
    if (y < x) x = y;
  }
  return x;
}

double bifrost::data::Manager::min(int arc, int triplet) const {
  double x{max(arc, triplet)};
  if (arc < 0 || arc >= arcs || triplet < 0 || triplet >= triplets) return x;
  for (auto t: TYPE1D){
    auto y = min_1D(arc, triplet, t);
    if (y < x) x = y;
  }
  for (auto t: TYPE2D){
    auto y = min_2D(arc, triplet, t);
    if (y < x) x = y;
  }
  return x;
}

double bifrost::data::Manager::min(int arc, int triplet, Type type) const {
  double x{max(arc, triplet, type)};
  if (Type::a == type || Type::b == type || Type::p == type || Type::x == type || Type::t == type){
    auto y = min_1D(arc, triplet, type);
    if (y < x) x = y;
  }
  if (Type::ab == type || Type::xp == type || Type::xt == type || Type::pt == type){
    auto y = min_2D(arc, triplet, type);
    if (y < x) x = y;
  }
  return x;
}

double bifrost::data::Manager::min(int arc, Type type) const {
  double x{max(arc, type)};
  if (Type::a == type || Type::b == type || Type::p == type || Type::x == type || Type::t == type){
    for (int t=0; t<triplets; ++t){
      auto y = min_1D(arc, t, type);
      if (y < x) x = y;
    }
  }
  if (Type::ab == type || Type::xp == type || Type::xt == type || Type::pt == type){
    for (int t=0; t<triplets; ++t){
      auto y = min_2D(arc, t, type);
      if (y < x) x = y;
    }
  }
  return x;
}




double bifrost::data::Manager::max_1D(int arc, int triplet, bifrost::data::Type t) const {
    double x{-1};
    auto key = std::make_tuple(arc, triplet, t);
    if (data_1d.count(key)){
        for (const auto & y: *data_1d.at(key)) if (y > x) x = y;
    }
    return x;
}

double bifrost::data::Manager::max_2D(int arc, int triplet, bifrost::data::Type t) const {
    double x{-1};
    auto key = std::make_tuple(arc, triplet, t);
    if (data_1d.count(key)){
        auto data = data_2d.at(key);
        for (int i=0; i<BIN2D; ++i){
            for (int j=0; j<BIN2D; ++j){
                if (data->cell(i, j) > x) x = data->cell(i, j);
            }
        }
    }
    return x;
}

double bifrost::data::Manager::min_1D(int arc, int triplet, bifrost::data::Type t) const {
  double x{max_1D(arc, triplet, t)};
  auto key = std::make_tuple(arc, triplet, t);
  if (data_1d.count(key)){
    for (const auto & y: *data_1d.at(key)) if (y < x) x = y;
  }
  return x;
}

double bifrost::data::Manager::min_2D(int arc, int triplet, bifrost::data::Type t) const {
  double x{max_2D(arc, triplet, t )};
  auto key = std::make_tuple(arc, triplet, t);
  if (data_1d.count(key)){
    auto data = data_2d.at(key);
    for (int i=0; i<BIN2D; ++i){
      for (int j=0; j<BIN2D; ++j){
        if (data->cell(i, j) < x) x = data->cell(i, j);
      }
    }
  }
  return x;
}

bifrost::data::Manager::D1 *bifrost::data::Manager::data_1D(int arc, int triplet, bifrost::data::Type t) const {
    auto key = std::make_tuple(arc, triplet, t);
    return data_1d.count(key) ? data_1d.at(key) : nullptr;
}

bifrost::data::Manager::D2 *bifrost::data::Manager::data_2D(int arc, int triplet, bifrost::data::Type t) const {
    auto key = std::make_tuple(arc, triplet, t);
    return data_2d.count(key) ? data_2d.at(key) : nullptr;
}


