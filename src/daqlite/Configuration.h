// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Configuration.h
///
/// \brief Daquiri light configuration parameters
///
/// Provides default values and allow loading from json file
//===----------------------------------------------------------------------===//

#pragma once

#include <types/PlotType.h>

#include <nlohmann/json.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

class Configuration {
public:
  /// \brief constructor using default values
  /// Default are likely to be unsuitable and this should probably
  /// always be followed by a call to fromJsonFile()
  Configuration() {}

  /// \brief loads configuration from JSON file
  void fromJsonObj(const nlohmann::json &obj);

  /// \brief loads configuration from JSON file
  void fromJsonFile(const std::string &path);

  /// \brief loads configuration from JSON file
  static std::vector<Configuration> getConfigurations(const std::string &path);

  static void prettyJSON(nlohmann::json &obj, const std::string &header = "",
                         int indent = 4);

  // get the Kafka related config options
  void getKafkaConfig();

  // get the Geometry related config options
  void getGeometryConfig();

  // get the Plot related config options
  void getPlotConfig();

  // get the TOF related config options
  void getTOFConfig();

  /// \brief prints the settings
  void print();

  /// \brief Read a typed value from the JSON configuration.
  ///
  /// Looks up \p Option inside \p Group. If found, returns the value cast to T.
  /// If not found and \p Throw is false, prints a warning and returns \p Default.
  /// If not found and \p Throw is true, throws std::runtime_error.
  ///
  /// \tparam T         Target type (bool, int, unsigned int, std::string, …)
  /// \param Group      Top-level JSON key (e.g. "kafka", "geometry")
  /// \param Option     Key within \p Group (e.g. "broker", "xdim")
  /// \param Default    Value to return when the option is absent
  /// \param Throw      If true, throw instead of returning the default
  template <typename T>
  T getVal(const std::string &Group, const std::string &Option, T Default,
           bool Throw = false);

  /// \brief Read an optional string value from the JSON configuration.
  ///
  /// Overload for \c std::optional<std::string> source fields. Returns the
  /// string value wrapped in an optional if the key is present, otherwise
  /// returns \p Default (typically \c std::nullopt). Never throws.
  ///
  /// \param Group    Top-level JSON key (e.g. "plot")
  /// \param Option   Key within \p Group (e.g. "source")
  /// \param Default  Value to return when the option is absent
  std::optional<std::string> getVal(const std::string &Group,
                                    const std::string &Option,
                                    std::optional<std::string> Default);

  // Configurable options
  struct TOFOptions {
    uint32_t Scale{1000};     // ns -> us
    uint32_t MaxValue{25000}; // us
    uint32_t BinSize{512};    // initial bin size
    bool AutoScaleX{true};
    bool AutoScaleY{true};
  };

  struct GeometryOptions {
    uint32_t XDim{1};
    uint32_t YDim{1};
    uint32_t ZDim{1};
    uint32_t Offset{0};
  };

  struct KafkaOptions {
    std::string Topic{"nmx_detector"};
    std::string Broker{"172.17.5.38:9092"};
    std::optional<std::string> Source{std::nullopt};
    std::string MessageMaxBytes{"10000000"};
    std::string FetchMessageMaxBytes{"10000000"};
    std::string ReplicaFetchMaxBytes{"10000000"};
    std::string EnableAutoCommit{"false"};
    std::string EnableAutoOffsetStore{"false"};
  };

  struct PlotOptions {
    PlotType Plot{PlotType::PIXELS}; // "tof" and "tof2d" are also possible
    bool ClearPeriodic{false};
    uint32_t ClearEverySeconds{5};
    bool Interpolate{false};
    std::string ColorGradient{"hot"};
    bool InvertGradient{false};
    bool LogScale{false};
    std::string WindowTitle{"Daquiri Lite - Daqlite"};
    std::string PlotTitle;
    std::string XAxis;
    std::optional<std::string> Source{std::nullopt};

    int Width{600};             // Default window width
    int Height{400};            // Default window height
    bool defaultGeometry{true}; // True if window geometries are default
  };

  TOFOptions mTOF;
  GeometryOptions mGeometry;
  KafkaOptions mKafka;
  PlotOptions mPlot;

  std::string mKafkaConfigFile;
  std::vector<std::pair<std::string, std::string>> mKafkaConfig;

  nlohmann::json mJsonObj;
};
