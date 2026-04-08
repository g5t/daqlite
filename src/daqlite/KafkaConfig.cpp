// Copyright (C) 2022 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file KafkaConfig.cpp
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <KafkaConfig.h>

#include <JsonFile.h>

#include <nlohmann/json.hpp>

#include <fmt/format.h>

#include <map>
#include <stdexcept>

using std::map;
using std::string;

KafkaConfig::KafkaConfig(const string &KafkaConfigFile) {
  if (KafkaConfigFile.empty()) {
    return;
  }
  fmt::print("KAFKA CONFIG FROM FILE\n");
  nlohmann::json root = from_json_file(KafkaConfigFile);

  try {
    nlohmann::json KafkaParms = root["KafkaParms"];

    for (const auto &Parm : KafkaParms) {
      map<string, string> MyMap = Parm;
      for (const auto &[key, value] : MyMap) {
        CfgParms.push_back({key, value});
      }
    }

  } catch (const std::exception &e) {
    const string Msg = fmt::format("Invalid Json file for Kafka config: {}", e.what());
    fmt::print("{}\n", Msg);
    throw std::runtime_error(Msg);
  }
}
