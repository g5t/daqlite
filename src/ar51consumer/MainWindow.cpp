// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.cpp
///
//===----------------------------------------------------------------------===//

//#include "ui_MainWindow.h"
#include <MainWindow.h>
#include <string.h>


MainWindow::MainWindow(std::string Broker, std::string Topic, QWidget *parent)
    : QMainWindow(parent) {
  startKafkaConsumerThread(Broker, Topic);
}

MainWindow::~MainWindow() {}

void MainWindow::startKafkaConsumerThread(std::string Broker, std::string Topic) {
  KafkaConsumerThread = new WorkerThread(Broker, Topic);
  KafkaConsumerThread->start();
}
