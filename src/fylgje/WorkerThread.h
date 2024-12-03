// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file WorkerThread.h
///
/// \brief main consumer loop for ar51consumer
//===----------------------------------------------------------------------===//

#pragma once

#include "ESSConsumer.h"
#include "KafkaConfig.h"
#include "Configuration.h"
#include <QThread>
#include <utility>

class WorkerThread : public QThread {
  Q_OBJECT

public:
  using data_t = ESSConsumer::data_t;
  WorkerThread(data_t * data, data_t * included, data_t * excluded, Configuration & Config):
  configuration(Config) {
    KafkaConfig kcfg(Config.KafkaConfigFile);
    Consumer = new ESSConsumer(data, included, excluded, configuration, kcfg.CfgParms);
  };

  /// \brief thread main loop
  void run() override;

  /// \brief Kafka consumer
  ESSConsumer *Consumer;

private:
  Configuration &configuration;

signals:
  /// \brief this signal is 'emitted' when there is new data
  void resultReady();

};
