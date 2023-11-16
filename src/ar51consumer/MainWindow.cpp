// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.cpp
///
//===----------------------------------------------------------------------===//

#include <MainWindow.h>
#include <fmt/format.h>
#include <string.h>


MainWindow::MainWindow(std::string Broker, std::string Topic, QWidget *parent)
    : QMainWindow(parent) {

  QVector<double> newx(64), newy(64), newy2(64);
  x = newx;
  y = newy;
  y2 = newy2;
  for (int xval = 0; xval < 64; xval ++) {
    x[xval] = xval;
  }

  layout = new QGridLayout;

  for (int Row = 0; Row < 3; Row++) {
    for (int Col = 0; Col < 5; Col++) {
      if (Row == 2 and Col == 4)
        continue;
      setupPlottingWidgets(Row, Col);
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


void MainWindow::setupPlottingWidgets(int Row, int Col) {
  auto QCP = new QCustomPlot();
  Graphs[Col][Row] = QCP;
  //std::string title = fmt::format("r{}, h{}", Row, Col);
  //QCP->legend->setVisible(true);
  QCP->xAxis->setRange(0, 63);
  QCP->xAxis2->setRange(0, 63);
  //QCP->yAxis->setRange(0, 1000);
  QCP->addGraph();
  //QCP->graph(0)->setName(title.c_str());
  QCP->graph(0)->setData(x, y);
  QCP->graph(0)->setLineStyle(QCPGraph::LineStyle::lsStepLeft);
  QCP->graph(0)->setBrush(QBrush(QColor(20,50,255,20)));
  QCP->addGraph();
  QCP->graph(1)->setData(x, y2);
  QCP->graph(1)->setLineStyle(QCPGraph::LineStyle::lsStepLeft);
  QCP->graph(1)->setBrush(QBrush(QColor(255,50,20,20)));
  layout->addWidget(QCP, Row, Col);
}

void MainWindow::startKafkaConsumerThread(std::string Broker, std::string Topic) {
  KafkaConsumerThread = new WorkerThread(Broker, Topic);

  connect(KafkaConsumerThread, &WorkerThread::resultReady, this,
          &MainWindow::handleReceivedData);

  KafkaConsumerThread->start();
}


void MainWindow::handleReceivedData() {

  for (int Row = 0; Row < 3; Row++) {
    for (int Col = 0; Col < 5; Col++) {
      if (Row == 2 and Col == 4)
        continue;
      int Ring = Row;
      int Hybrid = Col;
      auto qp = Graphs[Col][Row];
      for (int i= 0; i < 64; i++) {
        y[i] = KafkaConsumerThread->Consumer->Histogram[Ring][Hybrid][0][i];
        y2[i] = KafkaConsumerThread->Consumer->Histogram[Ring][Hybrid][1][i];
      }
      qp->graph(0)->setData(x, y);
      qp->graph(1)->setData(x, y2);
      qp->yAxis->rescale();
      qp->yAxis2->rescale();
      qp->replot();
    }
  }
}
