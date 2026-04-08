// Copyright (C) 2022 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file WorkerThread.cpp
///
/// \brief main consumer loop implementation for Daquiri Light (daqlite)
//===----------------------------------------------------------------------===//

#include <WorkerThread.h>

#include <Configuration.h>
#include <ESSConsumer.h>
#include <KafkaConfig.h>

#include <chrono>
#include <ratio>

WorkerThread::WorkerThread(Configuration &Config) {
  KafkaConfig KafkaCfg(Config.mKafkaConfigFile);
  Consumer = std::make_unique<ESSConsumer>(Config, KafkaCfg.CfgParms);
}

void WorkerThread::run() {
  auto t2 = std::chrono::high_resolution_clock::now();
  auto t1 = std::chrono::high_resolution_clock::now();

  while (!mStop) {
    auto Msg = Consumer->consume();

    Consumer->handleMessage(Msg.get());

    t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<int64_t, std::nano> elapsed = t2 - t1;

    // Once every ~1 second, copy the histograms and tell main thread
    // that plots can be updated.
    if (elapsed.count() >= 1000000000LL) {

      int ElapsedCountMS = elapsed.count()/1000000;
      emit resultReady(ElapsedCountMS);

      t1 = std::chrono::high_resolution_clock::now();
    }
  }
}

