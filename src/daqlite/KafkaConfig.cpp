// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
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
  if (KafkaConfigFile == "") {
    return;
  }
  fmt::print("KAFKA CONFIG FROM FILE\n");
  nlohmann::json root = from_json_file(KafkaConfigFile);

  try {
    nlohmann::json KafkaParms = root["KafkaParms"];

    for (const auto &Parm : KafkaParms) {
      map<string, string> MyMap = Parm;
      for (auto it = MyMap.begin(); it != MyMap.end(); it++) {
        std::pair<string, string> CfgPair{it->first, it->second};
        CfgParms.push_back(CfgPair);
      }
    }

  } catch (...) {
    fmt::print("Kafka JSON config - error: Invalid Json file: {}\n",
        KafkaConfigFile);
    throw std::runtime_error("Invalid Json file for Kafka config");
  }
}
