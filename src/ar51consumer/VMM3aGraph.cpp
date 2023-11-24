// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file VMM3aGraph
///
/// \brief Class for handling plotting of VMM readouts
///
//===----------------------------------------------------------------------===//

#include <QPlot/qcustomplot/qcustomplot.h>
#include <VMM3aGraph.h>
#include <map>


void VMM3aGraph::setupPlot(QGridLayout * Layout) {

  for (int i = 0; i < 64; i++) {
    x.push_back(i);
    y0.push_back(0);
    y1.push_back(0);
  }

  for (int Row = 0; Row < 3; Row++) {
    for (int Col = 0; Col < 5; Col++) {
      if (Row == 2 and Col == 4)
        continue;
      addGraph(Layout, Row, Col);
    }
  }

  // Add final row of buttons
  QPushButton *btnToggle = new QPushButton("Toggle");
  QPushButton *btnDead = new QPushButton("Dead");
  QPushButton *btnClear = new QPushButton("Clear");
  QPushButton *btnQuit = new QPushButton("Quit");

  QHBoxLayout *hblayout = new QHBoxLayout();
  hblayout->addWidget(btnToggle);
  hblayout->addWidget(btnDead);
  hblayout->addWidget(btnClear);
  hblayout->addWidget(btnQuit);

  Layout->addLayout(hblayout, 4, 0);

  connect(btnToggle, SIGNAL(clicked()), this, SLOT(toggle()));
  connect(btnDead, SIGNAL(clicked()), this, SLOT(dead()));
  connect(btnClear, SIGNAL(clicked()), this, SLOT(clear()));
  connect(btnQuit, SIGNAL(clicked()), this, SLOT(quitProg()));


  /// Update timer
  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &VMM3aGraph::updatePlots);
  timer->start(1000);
}


  /// \brief
void VMM3aGraph::addGraph(QGridLayout * Layout, int Row, int Col) {
  QCustomPlot * QCP = new QCustomPlot();
  int GraphKey = Row * 256 + Col;
  Graphs[GraphKey] = QCP;

  //std::string title = fmt::format("r{}, h{}", Row, Col);
  //QCP->legend->setVisible(true);
  QCP->xAxis->setRange(0, 63);
  QCP->yAxis->setRange(0, 5);
  QCP->addGraph();
  //QCP->graph(0)->setName(title.c_str());
  QCP->graph(0)->setData(x, y0);
  QCP->graph(0)->setLineStyle(QCPGraph::LineStyle::lsStepLeft);
  QCP->graph(0)->setBrush(QBrush(QColor(20,50,255,20)));
  QCP->addGraph();
  QCP->graph(1)->setData(x, y1);
  QCP->graph(1)->setLineStyle(QCPGraph::LineStyle::lsStepLeft);
  QCP->graph(1)->setBrush(QBrush(QColor(255,50,20,20)));
  Layout->addWidget(QCP, Row, Col);
}


void VMM3aGraph::updatePlots() {
  for (int Row = 0; Row < 3; Row++) {
    for (int Col = 0; Col < 5; Col++) {
      if (Row == 2 and Col == 4)
        continue;
      int Ring = Row;
      int Hybrid = Col;
      int GraphKey = Row * 256 + Col;
      auto qp = Graphs[GraphKey];

      for (int i= 0; i < 64; i++) {
        y0[i] = WThread->Consumer->Histogram[Ring][Hybrid][0][i];
        y1[i] = WThread->Consumer->Histogram[Ring][Hybrid][1][i];
      }

      qp->graph(0)->setVisible(false);
      qp->graph(1)->setVisible(false);
      if (TogglePlots == 0 or TogglePlots == 1) {
        qp->graph(0)->setVisible(true);
        qp->graph(0)->setData(x, y0);
      }
      if (TogglePlots == 0 or TogglePlots == 2) {
        qp->graph(1)->setVisible(true);
        qp->graph(1)->setData(x, y1);
      }

      qp->yAxis->rescale();
      qp->replot();
    }
  }
}


void VMM3aGraph::toggle() {
  TogglePlots ^= 1;
  updatePlots();
}


void VMM3aGraph::dead() {
  for (int Row = 0; Row < 3; Row++) {
    for (int Col = 0; Col < 5; Col++) {
      if (Row == 2 and Col == 4)
        continue;
      int Ring = Row;
      int Hybrid = Col;
      int DeadAsic0{0};
      int DeadAsic1{0};
      for (int i= 0; i < 64; i++) {
        if (WThread->Consumer->Histogram[Ring][Hybrid][0][i] == 0) {
          DeadAsic0++;
        }
        if (i >=16 and i <= 47 and WThread->Consumer->Histogram[Ring][Hybrid][0][i] == 0) {
          DeadAsic1++;
        }
      }
      qDebug("Ring %d, Hybrid %d - dead strips %d, dead wires %d", Ring, Hybrid, DeadAsic0, DeadAsic1);
    }
  }
}


void VMM3aGraph::clear() {
  memset(WThread->Consumer->Histogram, 0,
    sizeof(WThread->Consumer->Histogram));
  updatePlots();
}


void VMM3aGraph::quitProg() {
  QApplication::quit();
}
