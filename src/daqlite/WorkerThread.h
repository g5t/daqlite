// Copyright (C) 2020 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file WorkerThread.h
///
/// \brief main consumer loop for Daquiri Light (daqlite)
/// The worker thread continuously calls ESSConsumer::consume() and
/// ESSConsumer::handleMessage() to histogram the pixelids. Once every second
/// the plotting thread (qt main thread?) is
/// notified to update and plot new data.
//===----------------------------------------------------------------------===//

#pragma once

#include <ESSConsumer.h>

#include <QThread>

#include <atomic>
#include <memory>
#include <stdexcept>

class Configuration;

class WorkerThread : public QThread {
  Q_OBJECT

public:
  explicit WorkerThread(Configuration &Config);

  ~WorkerThread() {
    mStop = true;
    wait();
  }

  /// \brief thread main loop
  void run() override;

  /// \brief Getter for the consumer
  ESSConsumer &getConsumer() {
    if (!Consumer) {
      throw std::runtime_error("Consumer is not initialized");
    }
    return *Consumer;
  }

signals:
  /// \brief this signal is 'emitted' when there is new data to be plotted
  /// this is done periodically (approximately once every second)
  void resultReady(int &val);

private:
  /// \brief Set to true by the destructor to signal the run() loop to exit
  std::atomic<bool> mStop{false};

  /// \brief Kafka consumer
  std::unique_ptr<ESSConsumer> Consumer;
};
