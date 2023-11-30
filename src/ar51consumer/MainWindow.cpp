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


MainWindow::MainWindow(std::string Broker, std::string Topic,
  std::string Readout, QWidget *parent)
    : QMainWindow(parent) {

  if (Readout == "VMM") {
    vmmgraph.setupPlot(&layout);
  } else if (Readout == "CDT") {
    cdtgraph.setupPlot(&layout);
  } else {
    qDebug("Unknown readout type %s (Use one of VMM, CDT)", Readout.c_str());
    exit(0);
  }

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
  vmmgraph.WThread = Consumer;
  cdtgraph.WThread = Consumer;
  Consumer->start();
}
