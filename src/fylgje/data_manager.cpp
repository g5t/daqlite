#include <iostream>
#include <fmt/format.h>
#include <sstream>
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

int bifrost::data::Manager::group(int arc, int triplet) const {
  return arc * triplets + triplet;
}

///\brief Replicate the EFU calculations to identify a unique pixel number
///\returns 0 if no valid pixel
int bifrost::data::Manager::pixel(int arc, int triplet, int a, int b) const {
  auto g = group(arc, triplet);
  auto pos = static_cast<double>(a) / static_cast<double>(a + b);
  auto tube = calibration.getUnitId(g, pos);
  if (tube < 0) {
    // invalid global position (outside any unit's range)
    return 0;
  }
  auto cor_pos = calibration.posCorrection(g, tube, calibration.unitPosition(g, tube,  pos)); // in range (0, 1)
  // corrected position in (0.0, 1.0) is mapped to a tube pixel in (0, pixels_per_tube - 1)
  // its offset by which tube it is, which triplet its in, and which arc its in
  int offset = pixels_per_tube * arc + pixels_per_tube * triplet + pixels_per_tube_arc * tube;
  // and note that valid pixels index from 1 -- not 0.
  return 1 + offset + static_cast<int>((pixels_per_tube - 1) * cor_pos);
}

///\brief Determine if the charge division would give a pixel number, and if the the pulse height is within threshold
bool bifrost::data::Manager::includes(int arc, int triplet, int a, int b) const {
  auto g = group(arc, triplet);
  if (g < 0) return false;

  auto pos = static_cast<double>(a) / static_cast<double>(a + b);
  auto tube = calibration.getUnitId(g, pos);
  if (tube < 0) return false;

  auto unit_pos = calibration.unitPosition(g, tube, pos);
  if (unit_pos < 0 || unit_pos > 1) return false;

  return calibration.pulseHeightOK(g, tube, a+b);
}



bool bifrost::data::Manager::add(int fiber, int group, int a, int b, double time){
    auto arc_ = arc(group);
    auto triplet_ = triplet(fiber, group);
    if (arc_ < 0 || arc_ >= arcs || triplet_ < 0 || triplet_ >= triplets) {
        return false;
    }
    auto allowed = includes(arc_, triplet_, a, b);
    if (allowed) {
      if (auto p = pixel(arc_, triplet_, a, b); (p > 0 && p <= total_pixels)) {
        pixel_data[p - 1] += 1;
      }
    }

    bool ok{true};
    ok &= add_1D(arc_, triplet_, a, b, time, allowed);
    ok &= add_2D(arc_, triplet_, a, b, time, allowed);
    return ok;
}

bool bifrost::data::Manager::add_1D(int arc, int triplet, int a, int b, double time, bool allowed){
  auto t_a = std::make_pair(Type::a, hist_a_or_b(a, SHIFT1D, BIN1D));
  auto t_b = std::make_pair(Type::b, hist_a_or_b(b, SHIFT1D, BIN1D));
  auto t_p = std::make_pair(Type::p, hist_p(a+b, SHIFT1D, BIN1D));
  auto t_x = std::make_pair(Type::x, hist_x(a, b, BIN1D));
  auto t_t = std::make_pair(Type::t, hist_t(time, BIN1D));
  if (t_a.second < 0 || t_b.second < 0 || t_p.second < 0 || t_x.second < 0 || t_t.second < 0) {
      return false;
  }
  std::vector<std::pair<map_t<data_t> *, bool>> each{{{&everything, true}, {&included, allowed}, {&excluded, !allowed}}};
  for (auto & [data, tf]: each) {
    if (tf) {
      for (auto [t, h]: {t_a, t_b, t_p, t_x, t_t}) {
        auto this_key = this->key(arc, triplet, t);
        if (!data->count(this_key)) {
          std::stringstream ss;
          ss << t;
          fmt::print("arc={}, triplet={}, type={} does not exist in managed data\n", arc, triplet, ss.str());
          return false;
        }
        if (static_cast<size_t>(h) < data->at(this_key).size()) {
          data->at(this_key).at(h) += 1;
        } else {
          std::stringstream ss;
          ss << t;
          fmt::print("arc={}, triplet={}, type={} at a={}, b={} time={} given out of bound index {}\n", arc, triplet,
                     ss.str(), a, b, time, h);
        }
      }
    }
  }
  return true;
}

bool bifrost::data::Manager::add_2D(int arc, int triplet, int full_a, int full_b, double full_t, bool allowed){
  auto a = hist_a_or_b(full_a, SHIFT2D, BIN2D);
  auto b = hist_a_or_b(full_b, SHIFT2D, BIN2D);
  auto p = hist_p(full_a+full_b, SHIFT2D, BIN2D);
  auto x = hist_x(full_a, full_b, BIN2D);
  auto t = hist_t(full_t, BIN2D);
  if (a < 0 || b < 0 || p < 0 || x < 0 || t < 0) return false;
  std::vector<std::tuple<Type, int, int>> t_i_j {{Type::ab, a, b}, {Type::pt, p, t}, {Type::xt, x, t}, {Type::xp, x, p}};
  std::vector<std::pair<map_t<data_t> *, bool>> each{{{&everything, true}, {&included, allowed}, {&excluded, !allowed}}};
  for (auto & [data, tf]: each) {
    if (tf) {
      for (auto [y, i, j]: t_i_j) {
        auto this_key = this->key(arc, triplet, y);
        if (!data->count(this_key)) {
          std::stringstream ss;
          ss << y;
          fmt::print("arc={}, triplet={}, type={} does not exist in managed data\n", arc, triplet, ss.str());
          return false;
        }
        auto ij = i * BIN2D + j;
        if (static_cast<size_t>(ij) < data->at(this_key).size()) {
          data->at(this_key).at(ij) += 1;
        } else {
          std::stringstream ss;
          ss << y;
          fmt::print("arc={}, triplet={}, type={} at a={}, b={} time={} given out of bound index ({}, {})\n", arc,
                     triplet, ss.str(), full_a, full_b, time, i, j);
        }
      }
    }
  }
  return true;
}

double bifrost::data::Manager::max(Filter which) const {
  double x{std::numeric_limits<double>::lowest()};
  for (int a=0; a<arcs; ++a) if (auto y = max(a, which); y > x) x = y;
  return x;
}

double bifrost::data::Manager::max(int arc, Filter which) const {
  double x{std::numeric_limits<double>::lowest()};
  for (int t=0; t<triplets; ++t) if (auto y = max(arc, t, which); y > x) x = y;
  return x;
}

double bifrost::data::Manager::max(int arc, int triplet, Filter which) const {
  double x{std::numeric_limits<double>::lowest()};
  if (arc < 0 || arc >= arcs || triplet < 0 || triplet >= triplets) return x;
  for (auto t: TYPEND) if (auto y = max(arc, triplet, t, which); y > x) x = y;
  return x;
}

double bifrost::data::Manager::max(int arc, Type type, Filter which) const {
  double x{std::numeric_limits<double>::lowest()};
  for (int t=0; t<triplets; ++t)if (auto y = max(arc, t, type, which); y > x) x = y;
  return x;
}

double bifrost::data::Manager::max(int arc, int triplet, bifrost::data::Type t, Filter which) const {
  return max(key(arc, triplet, t), which);
}

double bifrost::data::Manager::max(bifrost::data::key_t k, Filter which) const {
  return is_1D(key_type(k)) ? max_1D(k, which) : max_2D(k, which);
}

double bifrost::data::Manager::min(Filter which) const {
  double x{(std::numeric_limits<double>::max)()};
  for (int a=0; a<arcs; ++a) if (auto y = min(a, which);y < x) x = y;
  return x;
}

double bifrost::data::Manager::min(int arc, Filter which) const {
  double x{(std::numeric_limits<double>::max)()};
  for (int t=0; t<triplets; ++t) if (auto y = min(arc, t, which); y < x) x = y;
  return x;
}

double bifrost::data::Manager::min(int arc, int triplet, Filter which) const {
  double x{(std::numeric_limits<double>::max)()};
  if (arc < 0 || arc >= arcs || triplet < 0 || triplet >= triplets) return x;
  for (auto t: TYPEND) if (auto y = min(arc, triplet, t, which); y < x) x = y;
  return x;
}

double bifrost::data::Manager::min(int arc, Type type, Filter which) const {
  double x{(std::numeric_limits<double>::max)()};
  for (int t=0; t<triplets; ++t) if (auto y = min(arc, t, type, which); y < x) x = y;
  return x;
}

double bifrost::data::Manager::min(int arc, int triplet, bifrost::data::Type t, Filter which) const {
  return min(key(arc, triplet, t), which);
}

double bifrost::data::Manager::min(bifrost::data::key_t k, Filter which) const {
  return is_1D(key_type(k)) ? min_1D(k, which) : min_2D(k, which);
}


bifrost::data::Manager::D1 bifrost::data::Manager::data_1D(int arc, int triplet, bifrost::data::Type t, Filter which) const {
  return data_1D(this->key(arc, triplet, t), which);
}

bifrost::data::Manager::D1 bifrost::data::Manager::data_1D(bifrost::data::key_t k, Filter which) const {
    auto d = bifrost::data::Manager::D1();
    auto this_type = key_type(k);
    const auto & data{which == Filter::none ? everything : which == Filter::positive ? included : excluded};
    if (data.count(k) && bins_1d.count(this_type)){
      auto full = data.at(k);
      auto bins = bins_1d.at(this_type);
      d.reserve(bins);
      for (int i=0; i<bins; ++i) d.push_back(0.);
      if (BIN1D == bins){
        for (int i=0; i<BIN1D; ++i){
          d[i] = full.at(i);
        }
      } else {
        auto r = BIN1D / bins; // since all bins are powers of two, this is as well
        for (int i=0; i < bins; ++i){
          for (int j=0; j < r; ++j){
            d[i] += full.at(i*r + j);
          }
          d[i] /= r;
        }
      }
    }
    return d;
}

bifrost::data::Manager::D2 * bifrost::data::Manager::data_2D(int arc, int triplet, bifrost::data::Type t, Filter which) const {
  return data_2D(key(arc, triplet, t), which);
}

bifrost::data::Manager::D2 * bifrost::data::Manager::data_2D(bifrost::data::key_t k, Filter which) const {
  // translate 2d to 1d axes
  auto this_type = key_type(k);
  auto [nx, ny] = bins_2D(this_type);
  int bx{BIN2D/nx/2}, by{BIN2D/ny/2};
  double norm{static_cast<double>(BIN2D)*static_cast<double>(BIN2D)/static_cast<double>(nx)/static_cast<double>(ny)};

  auto d = new ::bifrost::data::Manager::D2(nx, ny, QCPRange(bx, BIN2D-bx), QCPRange(by, BIN2D-by));
  d->fill(0);
  const auto & data{which == Filter::none ? everything : which == Filter::positive ? included : excluded};
  if (data.count(k)){
    auto full = data.at(k);
    if (nx == BIN2D && ny == BIN2D){
      for (int ix=0; ix < nx; ++ix){
        for (int iy=0; iy < ny; ++iy){
          d->setCell(ix, iy, full.at(ix * BIN2D + iy));
        }
      }
    } else {
      auto rx = BIN2D / nx;
      auto ry = BIN2D / ny;
      for (int ix=0; ix < nx; ++ix){
        for (int iy=0; iy < ny; ++iy){
          double tmp{0.};
          for (int jx=0; jx < rx; ++jx){
            for (int jy=0; jy < ry; ++jy){
              auto z = (ix * rx + jx) * BIN2D + (iy * ry + jy);
              tmp += full.at(z);
            }
          }
          d->setCell(ix, iy, tmp/norm);
        }
      }
    }
  }
  return d;
}


int bifrost::data::Manager::max_1D(bifrost::data::key_t k, Filter which) const {
  int value{std::numeric_limits<int>::lowest()};
  auto this_type = key_type(k);
  const auto & data{which == Filter::none ? everything : which == Filter::positive ? included : excluded};
  if (data.count(k) && bins_1d.count(this_type)){
    auto full = data.at(k);
    auto bins = bins_1d.at(this_type);
    if (BIN1D == bins){
      for (int i=0; i<BIN1D; ++i){
        if (full.at(i) > value) value = full.at(i);
      }
    } else {
      auto r = BIN1D / bins; // since all bins are powers of two, this is as well
      for (int i=0; i < bins; ++i){
        int tmp{0};
        for (int j=0; j < r; ++j){
          tmp += full.at(i*r + j);
        }
        tmp = tmp ? tmp/r ? tmp/r : 1 : 0;
        if (tmp > value) value = tmp;
      }
    }
  }
  return value;
}

int bifrost::data::Manager::max_2D(bifrost::data::key_t k, Filter which) const {
  auto [nx, ny] = bins_2D(key_type(k));
  int value{std::numeric_limits<int>::lowest()};
  double norm{static_cast<double>(BIN2D)*static_cast<double>(BIN2D)/static_cast<double>(nx)/static_cast<double>(ny)};
  const auto & data{which == Filter::none ? everything : which == Filter::positive ? included : excluded};
  if (data.count(k)){
    auto & full = data.at(k);
    if (nx == BIN2D && ny == BIN2D){
      for (int ix=0; ix < nx; ++ix){
        for (int iy=0; iy < ny; ++iy){
          if (full.at(ix * BIN2D + iy) > value) value = full.at(ix * BIN2D + iy);
        }
      }
    } else {
      auto rx = BIN2D / nx;
      auto ry = BIN2D / ny;
      for (int ix=0; ix < nx; ++ix){
        for (int iy=0; iy < ny; ++iy){
          int tmp{0};
          for (int jx=0; jx < rx; ++jx){
            for (int jy=0; jy < ry; ++jy){
              auto z = (ix * rx + jx) * BIN2D + (iy * ry + jy);
              tmp += full.at(z);
            }
          }
          if (tmp > value) value = tmp;
        }
      }
    }
  }
  return static_cast<int>(std::ceil(static_cast<double>(value)/norm));
}


int bifrost::data::Manager::min_1D(bifrost::data::key_t k, Filter which) const {
  auto this_type = key_type(k);
  int value{(std::numeric_limits<int>::max)()};
  const auto & data{which == Filter::none ? everything : which == Filter::positive ? included : excluded};
  if (data.count(k) && bins_1d.count(this_type)){
    auto full = data.at(k);
    auto bins = bins_1d.at(this_type);
    if (BIN1D == bins){
      for (int i=0; i<BIN1D; ++i){
        if (full.at(i) < value) value = full.at(i);
      }
    } else {
      auto r = BIN1D / bins; // since all bins are powers of two, this is as well
      for (int i=0; i < bins; ++i){
        int tmp{0};
        for (int j=0; j < r; ++j){
          tmp += full.at(i*r + j);
        }
        tmp = tmp ? tmp/r ? tmp/r : 1 : 0;
        if (tmp < value) value = tmp;
      }
    }
  }
  return value;
}

int bifrost::data::Manager::min_2D(bifrost::data::key_t k, Filter which) const {
  auto this_type = key_type(k);
  auto [nx, ny] = bins_2D(this_type);
  double norm{static_cast<double>(BIN2D)*static_cast<double>(BIN2D)/static_cast<double>(nx)/static_cast<double>(ny)};
  int value{(std::numeric_limits<int>::max)()};
  const auto & data{which == Filter::none ? everything : which == Filter::positive ? included : excluded};
  if (data.count(k)){
    auto full = data.at(k);
    if (nx == BIN2D && ny == BIN2D){
      for (int ix=0; ix < nx; ++ix){
        for (int iy=0; iy < ny; ++iy){
          if (full.at(ix * BIN2D + iy) < value) value = full.at(ix * BIN2D + iy);
        }
      }
    } else {
      auto rx = BIN2D / nx;
      auto ry = BIN2D / ny;
      for (int ix=0; ix < nx; ++ix){
        for (int iy=0; iy < ny; ++iy){
          int tmp{0};
          for (int jx=0; jx < rx; ++jx){
            for (int jy=0; jy < ry; ++jy){
              auto z = (ix * rx + jx) * BIN2D + (iy * ry + jy);
              tmp += full.at(z);
            }
          }
          if (tmp < value) value = tmp;
        }
      }
    }
  }
  return static_cast<int>(std::ceil(static_cast<double>(value)/norm));
}


bool bifrost::data::is_1D(bifrost::data::Type t) {
  using bifrost::data::TYPE1D;
  return std::find(std::begin(TYPE1D), std::end(TYPE1D), t) != std::end(TYPE1D);
}
bool bifrost::data::is_2D(bifrost::data::Type t) {
  using bifrost::data::TYPE2D;
  return std::find(std::begin(TYPE2D), std::end(TYPE2D), t) != std::end(TYPE2D);
}

std::ostream & operator<<(std::ostream & os, ::bifrost::data::Type type){
  using ::bifrost::data::Type;
  switch (type){
    case Type::a: {os << "I(A)";  break;}
    case Type::x: {os << "I(x)";  break;}
    case Type::p: {os << "I(p)";  break;}
    case Type::xp: {os << "I(x,p)";  break;}
    case Type::ab: {os << "I(A,B)";  break;}
    case Type::b: {os << "I(B)";  break;}
    case Type::xt: {os << "I(x,t)";  break;}
    case Type::pt: {os << "I(p,t)";  break;}
    case Type::t: {os << "I(t)";  break;}
    default: os << "unknown Type";
  }
  return os;
}
//
//bool bifrost::data::key_less(const bifrost::data::key_t & a, const bifrost::data::key_t & b){
//  return key_compare(a, b) == -1;
//}
//
//int bifrost::data::key_compare(const bifrost::data::key_t & a, const bifrost::data::key_t & b){
//  if (std::get<0>(a) < std::get<0>(b)) return -1;
//  if (std::get<0>(a) > std::get<0>(b)) return 1;
//  if (std::get<1>(a) < std::get<1>(b)) return -1;
//  if (std::get<1>(a) > std::get<1>(b)) return 1;
//  if (std::get<2>(a) < std::get<2>(b)) return -1;
//  if (std::get<2>(a) > std::get<2>(b)) return 1;
//  return 0;
//}