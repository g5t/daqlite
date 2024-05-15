// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.cpp
///
//===----------------------------------------------------------------------===//

#include <MainWindow.h>
#include <fmt/format.h>

#include <utility>


MainWindow::MainWindow(std::string Broker, std::string Topic, QWidget *parent) : QMainWindow(parent) {
  caengraph.setupPlot(&layout);

  //creating a QWidget, and setting the window as parent
  auto * widget = new QWidget();
  widget->setLayout(&layout);
  setCentralWidget(widget);

  show();

  startConsumer(std::move(Broker), std::move(Topic));
}

// MainWindow::~MainWindow() {}


void MainWindow::startConsumer(std::string Broker, std::string Topic) {
  Consumer = new WorkerThread(std::move(Broker), std::move(Topic));
  caengraph.WThread = Consumer;
  Consumer->start();
}
