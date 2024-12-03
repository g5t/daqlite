#include "Calibration.h"

CalibrationGroup::CalibrationGroup(int i, std::vector<CalibrationUnit> && els)
: index{i}, elements{std::move(els)} {
  // check that the provided elements have exclusive position ranges, and are sorted + all present
  check_sorted_index_is_iota(els, "unit");
  for (auto ptr = els.begin(); ptr != els.end() && ptr+1 != els.end(); ++ptr){
    auto nxt = ptr+1;
    if (ptr->max_x() > nxt->min_x()) {
      std::stringstream ss;
      ss << fmt::format("Units ({}, {}) ({}, {}) error!", ptr->min_x(), ptr->max_x(), nxt->min_x(), nxt->max_x());
      throw std::runtime_error(ss.str());
    }
  }
}

Calibration::Calibration(int group_count, int element_count)
    : version_{0}, date_{std::time({})}, info_{"generated"}, instrument_{"generated"} {
  // use the same element ranges for all groups
  double step = 1.0 / static_cast<double>(element_count);
  std::vector<double> edges(1, 0.);
  edges.reserve(element_count + 1);
  for (int i=0; i<element_count - 1; ++i){
    edges.push_back(edges.back() + step);
  }
  edges.push_back(1.0);
  groups_.reserve(group_count);
  auto make_elements = [&](){
    std::vector<CalibrationUnit> elements;
    elements.reserve(element_count);
    for (int j=0; j<element_count; ++j){
      elements.emplace_back(j, edges[j], edges[j + 1]);
    }
    return elements;
  };
  for (int i=0; i<group_count; ++i){
    groups_.emplace_back(i, make_elements());
  }
}
[[nodiscard]] std::string Calibration::date_str() const {
  char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
  std::strftime(std::data(timeString), std::size(timeString), "%FT%TZ", std::gmtime(&date_));
  return timeString;
}
void Calibration::set_date(const std::string & date_str) {
  struct std::tm tm{};
  char * const result = strptime(date_str.c_str(), "%Y-%m-%dT%TZ", &tm);
  if (result != date_str.c_str() + date_str.size()){
    throw std::runtime_error(fmt::format("Failed to parse UTC time from '{}'", date_str));
  }
  date_ = timegm(&tm);
}
void Calibration::set_groups(Groups groups) {
  // ensure consistent group sizes
  auto els = groups.empty() ? 0 : groups.front().size();
  if (std::any_of(groups.begin(), groups.end(), [els](const auto & p){return p.size() != els;})){
    auto message = fmt::format("Inconsistent element sized group(s)! All should match first group, {} elements", els);
    throw std::runtime_error(message);
  }
  // ensure the groups are sorted by their index
  std::sort(groups.begin(), groups.end(),
            [](const auto & a, const auto & b){return a.index < b.index;}
  );
  // and that all indexes are present
  check_sorted_index_is_iota(groups, "group");
  groups_ = std::move(groups);
}


[[nodiscard]] double Calibration::posCorrection(int group, int unit, double pos) const {
  double corrected = pos - groups_[group].elements[unit].position_correction(pos);
  return corrected < 0 ? 0 : corrected > 1 ? 1 : corrected;
}

[[nodiscard]] int Calibration::getUnitId(int group, double pos) const {
  if (group >= static_cast<int>(groups_.size())){
    return -1;
  }
  for (const auto & el: groups_[group].elements){
    if (el.contains(pos)) {
      return el.index;
    }
  }
  return -1;
}

[[nodiscard]] double Calibration::unitPosition(int group, int unit, double global_position) const {
  if (group >= static_cast<int>(groups_.size())){
    return -1;
  }
  if (unit >= static_cast<int>(groups_[group].elements.size())){
    return -1;
  }
  return groups_[group].elements[unit].unit_position(global_position);
}

[[nodiscard]] int Calibration::pulseHeightOK(int group, int unit, int pulse_height) const {
  if (group >= static_cast<int>(groups_.size())){
    return false;
  }
  if (unit >= static_cast<int>(groups_[group].elements.size())){
    return false;
  }
  return groups_[group].elements[unit].pulse_height_ok(pulse_height);
}


void to_json(nlohmann::json & j, const CalibrationUnit & el){
  auto jel = nlohmann::json{{"unit", el.index}, {"left", el.left}, {"right", el.right}};
  if (el.c0.has_value()) jel["c0"] = el.c0.value();
  if (el.c1.has_value()) jel["c1"] = el.c1.value();
  if (el.c2.has_value()) jel["c2"] = el.c2.value();
  if (el.c3.has_value()) jel["c3"] = el.c3.value();
  if (el.min.has_value()) jel["min"] = el.min.value();
  if (el.max.has_value()) jel["max"] = el.max.value();
  j = jel;
}

void from_json(const nlohmann::json & j, CalibrationUnit & el){
  auto jel = j;
  el.index = jel["unit"];
  el.left = jel["left"];
  el.right = jel["right"];
  el.c0 = jel.contains("c0") ? std::optional(jel["c0"].get<double>()) : std::nullopt;
  el.c1 = jel.contains("c1") ? std::optional(jel["c1"].get<double>()) : std::nullopt;
  el.c2 = jel.contains("c2") ? std::optional(jel["c2"].get<double>()) : std::nullopt;
  el.c3 = jel.contains("c3") ? std::optional(jel["c3"].get<double>()) : std::nullopt;
  el.min = jel.contains("min") ? std::optional(jel["min"].get<int>()) : std::nullopt;
  el.max = jel.contains("max") ? std::optional(jel["max"].get<int>()) : std::nullopt;
}

void to_json(nlohmann::json & j, const CalibrationGroup & gr){
  j = nlohmann::json {{"units", gr.elements}, {"group", gr.index}};
}
void from_json(const nlohmann::json & j, CalibrationGroup & gr){
  auto jgr = j;
  auto els = jgr["units"].get<std::vector<CalibrationUnit>>();
  std::sort(els.begin(), els.end(), [](const auto & a, const auto & b){return a.index < b.index;});
  gr.index = jgr["group"].get<int>();
  gr.elements = els;
}


void to_json(nlohmann::json & j, const Calibration & cal){
  j = nlohmann::json{{"Calibration", {
    {"version", cal.version()},
    {"date", cal.date_str()},
    {"info", cal.info()},
    {"instrument", cal.instrument()},
    {"groups", cal.group_count()},
    {"units", cal.element_count()},
    {"parameters", cal.groups()}}
  }};
}

void from_json(const nlohmann::json & j, Calibration & cal){
  auto jc = j["Calibration"];
  auto parameters = jc["parameters"].get<Calibration::Groups>();
  if (auto groups = jc["groups"].get<int>(); groups != static_cast<int>(parameters.size())){
    auto message = fmt::format("Expected {} groups but json specifies {} instead!", parameters.size(), groups);
    throw std::runtime_error(message);
  }
  auto els = parameters.empty() ? 0 : parameters.front().size();
  if (auto elements = jc["units"].get<int>(); elements != static_cast<int>(els)){
    auto message = fmt::format("Expected {} units per group but json specifies {} instead!", els, elements);
    throw std::runtime_error(message);
  }
  cal.set_version(jc["version"].get<int>());
  cal.set_date(jc["date"].get<std::string>());
  cal.set_info(jc["info"].get<std::string>());
  cal.set_instrument(jc["instrument"].get<std::string>());
  cal.set_groups(parameters);
}