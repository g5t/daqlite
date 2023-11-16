// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file WorkerThread.cpp
///
//===----------------------------------------------------------------------===//

#include <WorkerThread.h>

void WorkerThread::run() {
  qDebug("Entering main consumer loop\n");
  while (true) {
    auto Msg = Consumer->consume();
    Consumer->handleMessage(Msg);
    delete Msg;
    emit resultReady();
  }
}
