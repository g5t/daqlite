// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.cpp
///
//===----------------------------------------------------------------------===//

#include <MainWindow.h>
#include <fmt/format.h>
#include <stdlib.h>
#include <string.h>


MainWindow::MainWindow(std::string Broker, std::string Topic, QWidget *parent) : QMainWindow(parent) {
  caengraph.setupPlot(&layout);

  //creating a QWidget, and setting the window as parent
  QWidget * widget = new QWidget();
  widget->setLayout(&layout);
  setCentralWidget(widget);

  show();

  startConsumer(Broker, Topic);
}

MainWindow::~MainWindow() {}


void MainWindow::startConsumer(std::string Broker, std::string Topic) {
  Consumer = new WorkerThread(Broker, Topic);
  caengraph.WThread = Consumer;
  Consumer->start();
}
