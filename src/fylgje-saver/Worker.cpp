// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Worker.cpp
///
//===----------------------------------------------------------------------===//

#include "Worker.h"

void Worker::run() {
  Consumer->consumeFrom(from);
  Consumer->consumeUntil(to);

  ESSConsumer::Status intent{ESSConsumer::Status::Continue};
  while (intent != ESSConsumer::Status::Halt) {
    auto Msg = Consumer->consume();
    intent = Consumer->handleMessage(Msg);
    delete Msg;
    if (ESSConsumer::Status::Update == intent){
        intent = ESSConsumer::Status::Continue;
    }
  }
  std::cout << "Done consuming\n";
}

