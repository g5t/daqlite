// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file WorkerThread.h
///
/// \brief main consumer loop for ar51consumer
//===----------------------------------------------------------------------===//

#pragma once

#include <ESSConsumer.h>
#include <QThread>

class WorkerThread : public QThread {
  Q_OBJECT

public:
  WorkerThread(std::string Broker, std::string Topic) {
    Consumer = new ESSConsumer(Broker, Topic);
  };

  ~WorkerThread(){};

  /// \brief thread main loop
  void run() override;

  /// \brief Kafka consumer
  ESSConsumer *Consumer;

signals:
  /// \brief this signal is 'emitted' when there is new data
  void resultReady();

};
