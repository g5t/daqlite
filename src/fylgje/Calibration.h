#pragma once
#include <JsonFile.h>
#include <string>
#include <iomanip>
#include <optional>
#include <cstring>

template<class Obj> void check_sorted_index_is_iota(const std::vector<Obj>& vec, const std::string & name){
  std::vector<int> indexes(vec.size(), 0);
  std::iota(indexes.begin(), indexes.end(), 0);
  auto match = std::mismatch(vec.begin(), vec.end(), indexes.begin(),
                             [](const auto & g, const auto & i){return g.index ==i;});
  if (match.first != vec.end()){
    std::stringstream ss;
    ss << "Incorrect " << name << " group indexing!" << std::endl;
    ss << "[";
    if (match.first != vec.begin()){
      ss << "...,";
    }
    for (auto p = match.first; p != vec.end(); ++p){
      ss << p->index << ",";
    }
    ss.seekp(-1, std::stringstream::cur);
    ss << "] does not match\n[";
    if (match.second != indexes.begin()){
      ss << "...,";
    }
    for (auto p = match.second; p != indexes.end(); ++p){
      ss << *p << ",";
    }
    ss.seekp(-1, std::stringstream::cur);
    ss << "]\n";
    throw std::runtime_error(ss.str());
  }
}

class CalibrationUnit {
public:
  int index{-1};
  // pos(x) = (x - left) / (right - left)  -- assuming x between left and right
  double left{0}, right{1};
  // linear_pos(pos) = pos - (c0 + c1 pos + c2 pos^2 + c3 pos^3)
  std::optional<double> c0{std::nullopt}, c1{std::nullopt}, c2{std::nullopt}, c3{std::nullopt};
  // accept only ADC values with min <= pulse_height <= max
  std::optional<int> min{std::nullopt}, max{std::nullopt};

  CalibrationUnit() = default;
  CalibrationUnit(int i, double l, double r): index{i}, left{l}, right{r} {}

  [[nodiscard]] inline double min_x() const {return left < right ? left : right;}
  [[nodiscard]] inline double max_x() const {return left < right ? right : left;}
  ///\brief Calculate the correction polynomial value for the specified position
  ///\param x a value in the range (0, 1) inclusive
  ///\returns c0 + c1 * x + c2 * x^2 + c3 * x^3
  ///\note Since all coefficients are optional, any missing values are replaced by 0.
  [[nodiscard]] inline double position_correction(double x) const{
    return c0.value_or(0.) + x * (c1.value_or(0.) + x * (c2.value_or(0.) + x * (c3.value_or(0.))));
  }
  ///\brief Determine if this CalibrationUnit covers the provided position
  ///\param x a position within the containing Group
  ///\returns true if x lies within (left, right)
  [[nodiscard]] inline bool contains(double x) const {
    return (std::min(left, right) <= x) && (x <= std::max(left, right));
  }
  ///\brief Find the unit position for the specified global position
  ///\param global_position a value in the range (0, 1) inclusive, like for `contains`
  ///\returns (x - left) / (right - left) clamped to the range (0 ,1)
  ///\note It is allowed for the unit position and global position to have opposite senses
  ///      and no check is performed to ensure the global position 'belongs' to this Unit.
  [[nodiscard]] inline double unit_position(double global_position) const {
    auto x = (global_position - left) / (right - left);
    return x < 0 ? 0 : x > 1 ? 1 : x;
  }
  ///\brief Use the optional threshold values to decide if a pulse height value is acceptable
  ///\param pulse_height the total pulse height as provided elsewhere
  ///\returns false if less than the minimum or more than the maximum, otherwise true
  [[nodiscard]] inline bool pulse_height_ok(int pulse_height) const {
    if (min.has_value() && pulse_height < min.value()) return false;
    if (max.has_value() && pulse_height > max.value()) return false;
    return true;
  }
};

class CalibrationGroup {
public:
  int index{-1};
  std::vector<CalibrationUnit> elements;
  CalibrationGroup() = default;
  CalibrationGroup(int i, std::vector<CalibrationUnit> && els);
  [[nodiscard]] size_t size() const {return elements.size();}
};



class Calibration {
public:
  Calibration() = default;
  Calibration(int group_count, int element_count);

  [[nodiscard]] int version() const {return version_;}
  void set_version(int v) {version_ = v;}
  [[nodiscard]] std::time_t date() const {return date_;}
  void set_date() {date_ = std::time({});}
  void set_date(std::time_t t) {date_ = t;}
  [[nodiscard]] std::string date_str() const;
  void set_date(const std::string & date_str);
  [[nodiscard]] const std::string & info() const {return info_;}
  void set_info(const std::string & i) {info_ = i;}
  [[nodiscard]] const std::string & instrument() const {return instrument_;}
  void set_instrument(const std::string & i) {instrument_ = i;}

  using Groups = std::vector<CalibrationGroup>;
  [[nodiscard]] const Groups & groups() const {return groups_;}
  void set_groups(Groups groups);

  [[nodiscard]] size_t group_count() const {return groups_.size();}
  [[nodiscard]] size_t element_count() const {return groups_.empty() ? 0 : groups_.front().size();}

  [[nodiscard]] double posCorrection(int group, int unit, double pos) const;
  [[nodiscard]] int getUnitId(int group, double pos) const;
  [[nodiscard]] double unitPosition(int group, int unit, double global_position) const;
  [[nodiscard]] int pulseHeightOK(int group, int unit, int pulse_height) const;

  CalibrationUnit * unit_pointer(int group, int unit){
    return &(groups_[group].elements[unit]);
  }

private:
  int version_{};
  std::time_t date_{};
  std::string info_;
  std::string instrument_;
  Groups groups_;
};

void to_json(nlohmann::json & j, const CalibrationUnit & el);
void from_json(const nlohmann::json & j, CalibrationUnit & el);

void to_json(nlohmann::json & j, const CalibrationGroup & gr);
void from_json(const nlohmann::json & j, CalibrationGroup & gr);

void to_json(nlohmann::json & j, const Calibration & cal);
void from_json(const nlohmann::json & j, Calibration & cal);
