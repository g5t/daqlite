// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.cpp
///
//===----------------------------------------------------------------------===//

#include <MainWindow.h>
#include <string.h>


MainWindow::MainWindow(std::string Broker, std::string Topic, QWidget *parent)
    : QMainWindow(parent) {

  QVector<double> newx(64), newy(64);
  x = newx;
  y = newy;
  for (int xval = 0; xval < 64; xval ++) {
    x[xval] = xval;
  }

  layout = new QGridLayout;

  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 5; col++) {
      auto qp = new QCustomPlot();
      Graphs[col][row] = qp;
      qp->addGraph();
      qp->graph(0)->setData(x, y);
      qp->xAxis->setRange(0, 63);
      qp->yAxis->setRange(0, 1000);
      layout->addWidget(qp, row, col);
    }
  }

  //creating a QWidget, and setting the window as parent
  QWidget * widget = new QWidget();
  widget->setLayout(layout);
  setCentralWidget(widget);

  show();

  startKafkaConsumerThread(Broker, Topic);
}

MainWindow::~MainWindow() {}

void MainWindow::startKafkaConsumerThread(std::string Broker, std::string Topic) {
  KafkaConsumerThread = new WorkerThread(Broker, Topic);

  connect(KafkaConsumerThread, &WorkerThread::resultReady, this,
          &MainWindow::handleReceivedData);

  KafkaConsumerThread->start();
}


void MainWindow::handleReceivedData() {

  for (int Row = 0; Row < 3; Row++) {
    for (int Col = 0; Col < 5; Col++) {
      int Ring = Row;
      int Hybrid = Col;
      auto qp = Graphs[Col][Row];
      for (int i= 0; i < 64; i++) {
        y[i] = KafkaConsumerThread->Consumer->Histogram[Ring][Hybrid][0][i];
      }
      qp->graph(0)->setData(x, y);
      qp->yAxis->rescale();
      qp->replot();
    }
  }
}
