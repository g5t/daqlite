// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Common abstraction for Charge Division calibration (CDCalibration)
/// Implementing ideas from ownCloud folder
/// 'DM/detectors/02 instruments/01 common/03 Calibration/Charge division'
///
//===----------------------------------------------------------------------===//

#pragma once

#include <JsonFile.h>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

class CDCalibration {
public:
  using interval_t = std::pair<double, double>;
  using intervals_t = std::vector<interval_t>;
  using all_intervals_t = std::vector<intervals_t>;

  using threshold_t = int64_t;
  using threshold_limits_t = std::pair<threshold_t, threshold_t>;
  using thresholds_t = std::vector<threshold_limits_t>;
  using all_thresholds_t = std::vector<thresholds_t>;

  using coefficients_t = std::vector<double>;
  using polynomials_t = std::vector<coefficients_t>;
  using all_polynomials_t = std::vector<polynomials_t>;


  CDCalibration() = default;

  /// \brief this constructor just saves the detector name, used in
  /// unit tests.
  CDCalibration(std::string Name) : Name(Name){};

  /// \brief load json from file into the jsion root object, used
  /// in detector plugin.
  CDCalibration(std::string Name, std::string CalibrationFile);

  /// \brief parse the calibration and validate internal consistency
  /// \retval True if file is valid, else False.
  void parseCalibration();

  /// \brief apply the position correction
  /// \param Pos the uncorrected position along the charge division unit
  /// \param GroupIndex which group are we in
  /// \param UnitIndex which polynomial of the N (GroupSize)
  /// \return the corrected position
  double posCorrection(int GroupIndex, int UnitIndex, double Pos);

  /// \brief return the UnitId provided the Group and the Global position
  int getUnitId(int GroupIndex, double pos);

  /// \brief (x_min, x_max) per group member, per group
  all_intervals_t Intervals;

  /// \brief (lower, upper) threshold limits per group member, per group
  all_thresholds_t Thresholds;

  /// \brief coefficients are vectors of vectors of vectors
  all_polynomials_t Calibration;

  // Grafana Counters
  struct Stats {
    int64_t ClampLow{0};
    int64_t ClampHigh{0};
    int64_t GroupErrors{0};
    int64_t OutsideInterval{0};
  } Stats;

  struct {
    int Groups{0}; // {LOKI: tubes, BIFROST: triplets, MIRACLES: tube pairs}
    int LoadedGroups{0};
    int GroupSize{0}; // {LOKI: straws, BIFROST: tubes, MIRACLES: tubes}
  } Parms;

  struct Parameters {
    int groupindex{0};
    intervals_t intervals;
    thresholds_t thresholds;
    polynomials_t polynomials;
  };

  std::string ConfigFile{""};
  nlohmann::json root;

  int saveCalibration(const std::string& filename) const;

  std::vector<Parameters> parameters() const;

private:
  ///\brief log and trace then throw runtime exception
  void throwException(std::string Message);

  ///\brief Do an initial sanity check of the provided json file
  /// called from parseCaibration()
  void consistencyCheck();

  ///\brief Load the parameters into a suitable structure
  void loadCalibration();

  ///\brief validate that the supplied intervals are consistent
  ///\param Index groupindex used for error messages
  ///\param Parameter the parameter section object
  void validateIntervals(int Index, nlohmann::json Parameter);

  ///\brief helper function to validate points in an interval are within
  /// the unit interval.
  bool inUnitInterval(std::pair<double, double> &Pair);

  ///\brief validate that the suplied thresholds are consistent
  ///\param Index groupindex use for error messages
  ///\param Parameter the parameter section object
  void validateThresholds(int Index, nlohmann::json Parameter);

  ///\brief validate that the provided polynomial coefficients have the
  /// expected sizes. More checks can be added for example we could calculate
  /// how large a fraction of the unit interval would clamp to high or low
  /// values and complain if the fraction is too large.
  ///\param Index groupindex used for error messages
  ///\param Parameter the parameter section object
  void validatePolynomials(int Index, nlohmann::json Parameter);

  ///\brief helper function to check that the returned value is an object.
  /// \todo not torally sure when it is expected to be this. For example if
  /// the returned value can be parsed as a string it is not an object.
  /// might be removed in the future if not useful.
  nlohmann::json getObjectAndCheck(nlohmann::json JsonObject,
                                   std::string Property);

  std::string Name{""}; ///< Detector/instrument name prvided in constructor

  std::string Message; /// Used for throwing exceptions.

  // to enable round-trip serialization
  std::string Info;
public:
  [[nodiscard]] const std::string & name() const {return Name;}
  [[nodiscard]] const std::string & info() const {return Info;}
  void info(const std::string & new_info) {Info = new_info;}
  void info(std::string && new_info) {Info = std::move(new_info);}
  [[nodiscard]] int groups() const {return Parms.Groups;}
  [[nodiscard]] int groupsize() const {return Parms.GroupSize;}
};

  void to_json(nlohmann::json & j, const Caen::CDCalibration & calibration);
  void from_json(const nlohmann::json & j, Caen::CDCalibration & calibration);
  void to_json(nlohmann::json & j, const Caen::CDCalibration::Parameters & parameters);
  void from_json(const nlohmann::json & j, Caen::CDCalibration::Parameters & parameters);
}