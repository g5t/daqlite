// Copyright (C) 2020 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file PlotType.h
///
/// Used for characterizing different daqlite plots
//===----------------------------------------------------------------------===//

#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

class PlotType {
public:

  // Enum definition
  enum Types {
    NONE = 0x01,
    ANY = 0x02,
    TOF2D = 0x03,
    TOF = 0x04,
    PIXELS = 0x05,
    HISTOGRAM = 0x06
  };

  // Max and min enum values
  static constexpr int MIN = Types::NONE;
  static constexpr int MAX = Types::HISTOGRAM;

  // Construct from string
  PlotType(const std::string &name) {

    // Convert to lower case, so both "value" and "VALUE" will work
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });

    if (lower == "none") {
      mPlotType = Types::NONE;
    }

    else if (lower == "any") {
      mPlotType = Types::ANY;
    }

    else if (lower == "tof2d") {
      mPlotType = Types::TOF2D;
    }

    else if (lower == "tof") {
      mPlotType = Types::TOF;
    }

    else if (lower == "pixels") {
      mPlotType = Types::PIXELS;
    }

    else if (lower == "histogram") {
      mPlotType = Types::HISTOGRAM;
    }

    else {
      throw std::invalid_argument("Invalid PlotType string: " + name);
    }
  }

  // Construct from integer
  PlotType(const int value) {
    if (value >= MIN && value <= MAX) {
      mPlotType = static_cast<Types>(value);
    }

    else {
      throw std::invalid_argument("Invalid PlotType integer: " +
                                  std::to_string(value));
    }
  }

  std::string asString() const {
    std::string result = "";

    switch (mPlotType) {
      case Types::NONE:
        result = "NONE";
        break;

      case Types::ANY:
        result = "ANY";
        break;

      case Types::TOF2D:
        result = "TOF2D";
        break;

      case Types::TOF:
        result = "TOF";
        break;

      case Types::PIXELS:
        result = "PIXELS";
        break;

      case Types::HISTOGRAM:
        result = "HISTOGRAM";
        break;

      default:
        break;
    }

    return result;
  }

  static std::vector<int> types() {
    return std::vector<int>{
      Types::NONE,
      Types::ANY,
      Types::TOF2D,
      Types::TOF,
      Types::PIXELS,
      Types::HISTOGRAM
    };
  }

  operator int() const { return static_cast<int>(mPlotType); }

  bool operator==(const PlotType &other) const {
    return mPlotType == other.mPlotType;
  }

  bool operator!=(const PlotType &other) const {
    return mPlotType != other.mPlotType;
  }

  bool operator==(const Types &other) const {
    return mPlotType == other;
  }

  bool operator!=(const Types &other) const {
    return mPlotType != other;
  }

private:
  Types mPlotType;
};
