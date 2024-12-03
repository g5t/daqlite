// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of Charge Division calibration (CDCalibration)
/// Implementing ideas from ownCloud folder
/// 'DM/detectors/02 instruments/01 common/03 Calibration/Charge division'
///
/// \brief using nlohmann json to parse calibrations read from file
//===----------------------------------------------------------------------===//

//#include <common/debug/Log.h>
//#include <common/debug/Trace.h>
#include <CDCalibration.h>
#include <Interval.h>
#include <utility>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

/// loads calibration from file
CDCalibration::CDCalibration(std::string Name, std::string CalibrationFile)
    : Name(Name) {

//  LOG(INIT, Sev::Info, "Loading calibration file {}", CalibrationFile);

  try {
    root = from_json_file(CalibrationFile);
  } catch (const std::exception & ex) {
    Message = fmt::format("Caen calibration error: {}", ex.what());
    throwException(Message);
  }
}

double CDCalibration::posCorrection(int Group, int Unit, double Pos) {
  std::vector<double> &Pols = Calibration[Group][Unit];
  double a = Pols[0];
  double b = Pols[1];
  double c = Pols[2];
  double d = Pols[3];

  double Delta = a + Pos * (b + Pos * (c + Pos * d));
//  XTRACE(EVENT, DEB, "group %d, unit %d, pos: %g, delta %g", Group, Unit, Pos,
//         Delta);
  double CorrectedPos = Pos - Delta;
//  XTRACE(EVENT, DEB, "CorrectedPos %g", CorrectedPos);

  if (CorrectedPos < 0.0) {
//    XTRACE(EVENT, INF, "Clamping to low value, pos: %g, delta %g", Pos, Delta);
    Stats.ClampLow++;
    CorrectedPos = 0.0;
  }
  if (CorrectedPos > 1.0) {
//    XTRACE(EVENT, INF, "Clamping to high value, pos: %g, delta %g", Pos, Delta);
    Stats.ClampHigh++;
    CorrectedPos = 1.0;
  }

  return CorrectedPos;
}

///\brief
int CDCalibration::getUnitId(int GroupIndex, double GlobalPos) {
//  XTRACE(EVENT, DEB, "GroupIndex %u GlobalPos %f", GroupIndex, GlobalPos);

  if (GroupIndex >= Parms.Groups) {
//    XTRACE(EVENT, WAR, "Provided GroupIndex %d > config (%d)", GroupIndex, Parms.Groups);
    Stats.GroupErrors++;
    return -1;
  }
  auto &GroupIntervals = Intervals[GroupIndex];

  int Unit;
  for (Unit = 0; Unit < (int)GroupIntervals.size(); Unit++) {
    double Min =
        std::min(GroupIntervals[Unit].first, GroupIntervals[Unit].second);
    double Max =
        std::max(GroupIntervals[Unit].first, GroupIntervals[Unit].second);
    if ((GlobalPos >= Min) and (GlobalPos <= Max)) {
      return Unit;
    }
  }
  Stats.OutsideInterval++;
  return -1;
}

///\brief Use a two-pass approach. One pass to validate as much as possible,
/// then a second pass to populate calibration table
void CDCalibration::parseCalibration() {
  consistencyCheck(); // first pass for checking
  loadCalibration();  // second pass to populate table
}

void CDCalibration::consistencyCheck() {
//  XTRACE(INIT, DEB, "Get Calibration object");
  nlohmann::json Calibration = getObjectAndCheck(root, "Calibration");
//  XTRACE(INIT, DEB, "Get instrument name");
  std::string Instrument = Calibration["instrument"];
  if (Instrument != Name) {
    throwException("Instrument name error");
  }

  Parms.Groups = Calibration["groups"];
  Parms.GroupSize = Calibration["groupsize"];
  info(Calibration["info"].get<std::string>());

//  XTRACE(INIT, ALW, "Parms: %s, %d, %d", Name.c_str(), Parms.Groups,
//         Parms.GroupSize);

//  XTRACE(INIT, DEB, "Get Parameter object");
  auto ParameterVector = Calibration["Parameters"];
  if (ParameterVector.size() != (unsigned int)(Parms.Groups)) {
    throw std::runtime_error(
        fmt::format("Calibration table error: expected {} entries, got {}",
                    Parms.Groups, ParameterVector.size()));
  }

  int Index{0};
  for (auto &Parm : ParameterVector) {
//    XTRACE(INIT, DEB, "Get groupindex object %d", Index);
    int GroupIndex = Parm["groupindex"];
    if (GroupIndex != Index) {
      Message =
          fmt::format("Index error: expected {}, got {}", Index, GroupIndex);
      throwException(Message);
    }
    validateIntervals(Index, Parm);
    validateThresholds(Index, Parm);
    validatePolynomials(Index, Parm);
    Index++;
  }
}

void CDCalibration::loadCalibration() {
  auto ParameterVector = root["Calibration"]["Parameters"];
  if ((int)ParameterVector.size() != Parms.Groups) {
    Message = fmt::format("Groupsize mismatch: {} specified, {} received",
      Parms.Groups, ParameterVector.size());
    throwException(Message);
  }

  Intervals.reserve(ParameterVector.size());
  Thresholds.reserve(ParameterVector.size());
  Calibration.reserve(ParameterVector.size());

  for (auto &Group : ParameterVector) {
    Intervals.push_back(Group["intervals"].get<intervals_t>());

    if (Group.contains("thresholds")){
      Thresholds.push_back(Group["thresholds"].get<thresholds_t>());
    } else {
      thresholds_t identity_thresholds;
      for (size_t i=0; i<Intervals.back().size(); ++i){
        identity_thresholds.emplace_back(threshold_t(0), (std::numeric_limits<threshold_t>::max)());
      }
      Thresholds.push_back(identity_thresholds);
    }
    if (Group.contains("polynomials")){
      Calibration.push_back(Group["polynomials"].get<polynomials_t>());
    } else {
      polynomials_t identity_polynomials;
      for (size_t i=0; i<Intervals.back().size(); ++i){
        identity_polynomials.push_back({0., 1., 0., 0.});
      }
      Calibration.push_back(identity_polynomials);
    }
  }
}

int CDCalibration::saveCalibration(const std::string& filename) const {
  nlohmann::json data = *this;
  to_json_file(data, filename);
  return 0;
}

nlohmann::json CDCalibration::getObjectAndCheck(nlohmann::json JsonObject,
                                                std::string Property) {
  nlohmann::json JsonObj = JsonObject[Property];
  if (not JsonObj.is_object()) {
    Message = fmt::format("'{}' does not return a json object", Property);
    throwException(Message);
  }
  return JsonObj;
}

void CDCalibration::validateIntervals(int Index, nlohmann::json Parameter) {
  intervals_t intervals = Parameter["intervals"];
  if (intervals.size() != (unsigned int)(Parms.GroupSize)) {
    Message = fmt::format(
        "Groupindex {} - interval array error: expected {} entries, got {}",
        Index, Parms.GroupSize, intervals.size());
    throwException(Message);
  }

  // check for interval overlaps
  if (Interval::overlaps(intervals)) {
    std::stringstream s;
    for (const auto & [lower, upper]: intervals){
      s << "[" << lower << ", " << upper << "], ";
    }
    Message = fmt::format("Groupindex {} has overlapping intervals {}", Index, s.str());
    throwException(Message);
  }

  int IntervalIndex{0};

  for (auto & interval : intervals) {
    if (not inUnitInterval(interval)) {
      Message =
          fmt::format("Groupindex {}, Intervalpos {} - bad range [{}; {}]",
                      Index, IntervalIndex, interval.first, interval.second);
      throwException(Message);
    }
    IntervalIndex++;
  }
}

void CDCalibration::validateThresholds(int Index, nlohmann::json Parameter) {
  using fmt::format;
  if (!Parameter.contains("thresholds")) return;

  thresholds_t thresholds = Parameter["thresholds"];
  if (thresholds.size() != static_cast<size_t>(Parms.GroupSize)) {
    Message = format("Groupindex {} - interval array error: expected {} entries, got {}",
                     Index, Parms.GroupSize, thresholds.size());
    throwException(Message);
  }
  int position{0};
  for (auto & [lower, upper]: thresholds){
    if (lower < 0){
      Message = fmt::format("Groupindex {}, Threshold position {} - bad lower threshold {}", Index, position, lower);
      throwException(Message);
    }
    /*
     * A too-high threshold doesn't _really_ matter, right? So skip this check for now.
     *
    // Thus far all CAEN firmware outputs A, B, C & D as 16-bit integers, but only using 15-bits.
    // It's not clear if the 16-th is a sign bit, an error flag, or strictly unused.
    // 4 * (2^16-1) is therefore the maximum threshold that makes sense, e.g., 1<<18 - 4
    if (upper > (1l<<18)){
      Message = fmt::format("Groupindex {}, Threshold position {} - bad upper threshold {}", Index, position, upper);
      throwException(Message);
    }
    */
    if (upper <= lower){
      Message = fmt::format("Groupindex {}, Threshold position {} - bad threshold range [{}, {}]", Index, position, lower, upper);
      throwException(Message);
    }
    ++position;
  }
}


void CDCalibration::validatePolynomials(int Index, nlohmann::json Parameter) {
  if (!Parameter.contains("polynomials")) return;

  std::vector<std::vector<double>> Polynomials = Parameter["polynomials"];
  if (Polynomials.size() != (unsigned int)Parms.GroupSize) {
    Message = fmt::format("Groupindex {} bad groupsize: expected {}, got {}",
                          Index, Parms.GroupSize, Polynomials.size());
    throwException(Message);
  }
  for (auto &Coefficients : Polynomials) {
    if (Coefficients.size() != 4) {
      Message =
          fmt::format("Groupindex {} coefficient error: expected 4, got {}",
                      Index, Coefficients.size());
      throwException(Message);
    }
  }
}

bool CDCalibration::inUnitInterval(std::pair<double, double> &Pair) {
  return ((Pair.first >= 0.0) and (Pair.first <= 1.0) and
          (Pair.second >= 0.0) and (Pair.second <= 1.0));
}

void CDCalibration::throwException(std::string Message) {
//  XTRACE(INIT, ERR, "%s", Message.c_str());
//  LOG(INIT, Sev::Error, Message);
  throw std::runtime_error(Message);
}

std::vector<CDCalibration::Parameters> CDCalibration::parameters() const {
  std::vector<CDCalibration::Parameters> pars;
  pars.reserve(Parms.Groups);
  for (int i=0; i<Parms.Groups; ++i) {
    pars.push_back({i, Intervals[i], Thresholds[i], Calibration[i]});
  }
  return pars;
}

} // namespace Caen


void Caen::to_json(nlohmann::json & j, const Caen::CDCalibration & calibration){
  // UTC RFC 3339, from https://en.cppreference.com/w/cpp/chrono/c/strftime
  std::time_t time = std::time({});
  char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
  std::strftime(std::data(timeString), std::size(timeString), "%FT%TZ", std::gmtime(&time));
  auto parameters = calibration.parameters();
  j = nlohmann::json{{
    "Calibration", {
      {"version", 0},
      {"date", timeString},
      {"info", calibration.info()},
      {"instrument", calibration.name()},
      {"groups", calibration.groups()},
      {"groupsize", calibration.groupsize()},
      {"Parameters", parameters}
    }
  }};
}

void Caen::to_json(nlohmann::json & j, const Caen::CDCalibration::Parameters & parameters){
  j = nlohmann::json{
    {"groupindex", parameters.groupindex},
    {"intervals", parameters.intervals},
    {"thresholds", parameters.thresholds},
    {"polynomials", parameters.polynomials}
  };
}

