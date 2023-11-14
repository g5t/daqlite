// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file WorkerThread.cpp
///
//===----------------------------------------------------------------------===//

#include <WorkerThread.h>

void WorkerThread::run() {

  while (true) {
    auto Msg = Consumer->consume();
    Consumer->handleMessage(Msg);
    delete Msg;
  }
}
