// Copyright (C) 2020 - 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Configuration.cpp
///
//===----------------------------------------------------------------------===//

#include "Configuration.h"
#include <fmt/format.h>

void Configuration::fromJsonFile(std::string fname) {
  std::ifstream ifs(fname, std::ofstream::in);
  if (!ifs.good()) {
    throw(std::runtime_error("Unable to create ifstream (bad filename?), exiting ..."));
  }

  try {
    ifs >> JsonObj;
  } catch (...) {
    throw(std::runtime_error("File is not valid JSON"));
  }

//  getGeometryConfig();
  getKafkaConfig();
  getPlotConfig();
//  getTOFConfig();
  print();
}
//
//void Configuration::getGeometryConfig() {
//  /// 'geometry' field is mandatory
//  Geometry.XDim = getVal("geometry", "xdim", Geometry.XDim, true);
//  Geometry.YDim = getVal("geometry", "ydim", Geometry.YDim, true);
//  Geometry.ZDim = getVal("geometry", "zdim", Geometry.ZDim, true);
//  Geometry.Offset = getVal("geometry", "offset", Geometry.Offset);
//}

void Configuration::getKafkaConfig() {
  /// 'broker' and 'topic' must be specified
  using std::operator""s;
  Kafka.Broker = getVal("kafka", "broker", "n/a"s, true);
  Kafka.Topic = getVal("kafka", "topic", "n/a"s, true);
  /// The rest are optional, using default values
  Kafka.MessageMaxBytes =
      getVal("kafka", "message.max.bytes", Kafka.MessageMaxBytes);
  Kafka.FetchMessagMaxBytes =
      getVal("kafka", "fetch.message.max.bytes", Kafka.FetchMessagMaxBytes);
  Kafka.ReplicaFetchMaxBytes =
      getVal("kafka", "replica.fetch.max.bytes", Kafka.ReplicaFetchMaxBytes);
  Kafka.EnableAutoCommit =
      getVal("kafka", "enable.auto.commit", Kafka.EnableAutoCommit);
  Kafka.EnableAutoOffsetStore =
      getVal("kafka", "enable.auto.offset.store", Kafka.EnableAutoOffsetStore);
}

void Configuration::getPlotConfig() {
  // Plot options - all are optional
  Plot.ClearPeriodic = getVal("plot", "clear_periodic", Plot.ClearPeriodic);
  Plot.ClearEverySeconds =
      getVal("plot", "clear_interval_seconds", Plot.ClearEverySeconds);
  Plot.ColorGradient = getVal("plot", "color_gradient", Plot.ColorGradient);
  Plot.InvertGradient = getVal("plot", "invert_gradient", Plot.InvertGradient);
  Plot.LogScale = getVal("plot", "log_scale", Plot.LogScale);

  // Window options - all are optional
  Plot.WindowTitle = getVal("plot", "window_title", Plot.WindowTitle);
  Plot.Width = getVal("plot", "window_width", Plot.Width);
  Plot.Height = getVal("plot", "window_height", Plot.Height);
}

//void Configuration::getTOFConfig() {
//  TOF.Scale = getVal("tof", "scale", TOF.Scale);
//  TOF.MaxValue = getVal("tof", "max_value", TOF.MaxValue);
//  TOF.BinSize = getVal("tof", "bin_size", TOF.BinSize);
//  TOF.AutoScaleX = getVal("tof", "auto_scale_x", TOF.AutoScaleX);
//  TOF.AutoScaleY = getVal("tof", "auto_scale_y", TOF.AutoScaleY);
//}

void Configuration::print() {
  fmt::print("[Kafka]\n");
  fmt::print("  broker {}\n", Kafka.Broker);
  fmt::print("  topic {}\n", Kafka.Topic);
  fmt::print("  message.max.bytes {}\n", Kafka.MessageMaxBytes);
  fmt::print("  fetch.message.max.bytes {}\n", Kafka.FetchMessagMaxBytes);
  fmt::print("  replica.fetch.max.bytes {}\n", Kafka.ReplicaFetchMaxBytes);
  fmt::print("  enable.auto.commit {}\n", Kafka.EnableAutoCommit);
  fmt::print("  enable.auto.offset.store {}\n", Kafka.EnableAutoOffsetStore);
//  fmt::print("[Geometry]\n");
//  fmt::print("  Dimensions ({}, {}, {})\n", Geometry.XDim, Geometry.YDim,
//             Geometry.ZDim);
//  fmt::print("  Pixel Offset {}\n", Geometry.Offset);
  fmt::print("[Plot]\n");
  fmt::print("  WindowTitle {}\n", Plot.WindowTitle);
  fmt::print("  Clear periodically {}\n", Plot.ClearPeriodic);
  fmt::print("  Clear interval (s) {}\n", Plot.ClearEverySeconds);
  fmt::print("  Color gradient {}\n", Plot.ColorGradient);
  fmt::print("  Invert gradient {}\n", Plot.InvertGradient);
  fmt::print("  Log Scale {}\n", Plot.LogScale);
//  fmt::print("[TOF]\n");
//  fmt::print("  Scale {}\n", TOF.Scale);
//  fmt::print("  Max value {}\n", TOF.MaxValue);
//  fmt::print("  Bin size {}\n", TOF.BinSize);
//  fmt::print("  Auto scale x {}\n", TOF.AutoScaleX);
//  fmt::print("  Auto scale y {}\n", TOF.AutoScaleY);
}

//\brief getVal() template is used to effectively achieve
// getInt(), getString() and getBool() functionality through T
template <typename T>
T Configuration::getVal(std::string Group, std::string Option, T Default,
                        bool Throw) {
  T ConfigVal;
  try {
    ConfigVal = JsonObj[Group][Option];
  } catch (nlohmann::json::exception &e) {
    fmt::print("Missing [{}][{}] configuration\n", Group, Option);
    if (Throw) {
      throw std::runtime_error("Daqlite config error");
    } else {
      fmt::print("Using default: {}\n", Default);
      return Default;
    }
  }
  return ConfigVal;
}
