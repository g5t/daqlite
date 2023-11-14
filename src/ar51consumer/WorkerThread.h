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
  WorkerThread(std::string Broker, std::string Topic)
    : KafkaBroker(Broker), KafkaTopic(Topic) {
    Consumer = new ESSConsumer(KafkaBroker, KafkaTopic);
  };

  ~WorkerThread(){};

  /// \brief thread main loop
  void run() override;

signals:
  /// \brief this signal is 'emitted' when there is new data to be plotted
  /// this is done periodically (approximately once every second)
  void resultReady(int &val);

private:
  /// \brief Kafka consumer
  ESSConsumer *Consumer;
  std::string KafkaBroker{""};
  std::string KafkaTopic{""};
};
