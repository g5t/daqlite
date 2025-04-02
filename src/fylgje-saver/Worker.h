// Copyright (C) 2023-2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Worker.h
///
/// \brief main consumer loop for ar51consumer
//===----------------------------------------------------------------------===//

#pragma once

#include "ESSConsumer.h"
#include "KafkaConfig.h"
#include "Configuration.h"
#include <utility>

class Worker {

public:
  using data_t = ESSConsumer::data_t;
  Worker(data_t * data, Configuration & Config, int64_t from, int64_t to):
  configuration(Config) {
    KafkaConfig kcfg(Config.KafkaConfigFile);
    Consumer = new ESSConsumer(data, configuration, kcfg.CfgParms);
//    Consumer->consumeAll();
//    Consumer->consumeFrom(from);
    Consumer->consumeUntil(to);
  };

  ~Worker(){
    delete Consumer;
  }

  /// \brief thread main loop
  void run();

  /// \brief Kafka consumer
  ESSConsumer *Consumer;

private:
  Configuration &configuration;
};
