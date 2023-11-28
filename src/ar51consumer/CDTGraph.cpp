// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file CDTGraph
///
/// \brief Class for handling plotting of CDT readouts
///
//===----------------------------------------------------------------------===//

#include <QPlot/qcustomplot/qcustomplot.h>
#include <CDTGraph.h>
#include <map>


bool ignoreRowCol(int Ring, int FEN) {
  std::vector<int> FENS {12, 10, 10, 6, 6, 6, 12, 8, 9, 9, 9};
  //if (FEN >= FENS[Ring]) {
  if ((FEN >= 4) or (Ring > 4)) {
    return true;
  }
  return false;
}


void CDTGraph::setupPlot(QGridLayout * Layout) {

  for (int i = 0; i < 256; i++) {
    x.push_back(i);
    y0.push_back(0);
    y1.push_back(0);
  }

  for (int Row = 0; Row < 11; Row++) {
    for (int Col = 0; Col < 12; Col++) {
      if (ignoreRowCol(Row, Col)) {
        continue;
      }
      addGraph(Layout, Row, Col);
    }
  }

  // Add final row of buttons
  QPushButton *btnToggle = new QPushButton("Select data");
  QPushButton *btnLogLin = new QPushButton("Log/lin");
  QPushButton *btnDead = new QPushButton("Dead");
  QPushButton *btnClear = new QPushButton("Clear");
  QPushButton *btnQuit = new QPushButton("Quit");

  QHBoxLayout *hblayout = new QHBoxLayout();
  hblayout->addWidget(btnToggle);
  hblayout->addWidget(btnLogLin);
  hblayout->addWidget(btnDead);
  hblayout->addWidget(btnClear);
  hblayout->addWidget(btnQuit);

  Layout->addLayout(hblayout, 11, 0);

  connect(btnToggle, SIGNAL(clicked()), this, SLOT(toggle()));
  connect(btnLogLin, SIGNAL(clicked()), this, SLOT(loglin()));
  connect(btnDead, SIGNAL(clicked()), this, SLOT(dead()));
  connect(btnClear, SIGNAL(clicked()), this, SLOT(clear()));
  connect(btnQuit, SIGNAL(clicked()), this, SLOT(quitProg()));


  /// Update timer
  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &CDTGraph::updatePlots);
  timer->start(1000);
}


  /// \brief
void CDTGraph::addGraph(QGridLayout * Layout, int Row, int Col) {
  QCustomPlot * QCP = new QCustomPlot();
  int Ring = Row;
  int FEN = Col;
  int GraphKey = Ring * 256 + FEN;
  Graphs[GraphKey] = QCP;

  //std::string title = fmt::format("r{}, h{}", Row, Col);
  //QCP->legend->setVisible(true);
  QCP->xAxis->setRange(0, 255);
  QCP->yAxis->setRange(0, 5);
  QCP->addGraph();
  //QCP->graph(0)->setName(title.c_str());
  QCP->graph(0)->setData(x, y0);
  QCP->graph(0)->setLineStyle(QCPGraph::LineStyle::lsStepLeft);
  QCP->graph(0)->setBrush(QBrush(QColor(20,50,255,40)));
  QCP->addGraph();
  QCP->graph(1)->setData(x, y1);
  QCP->graph(1)->setLineStyle(QCPGraph::LineStyle::lsStepLeft);
  QCP->graph(1)->setBrush(QBrush(QColor(255,50,20,40)));
  Layout->addWidget(QCP, Ring, FEN);
}


void CDTGraph::updatePlots() {
  for (int Row = 0; Row < 11; Row++) {
    for (int Col = 0; Col < 12; Col++) {
      if (ignoreRowCol(Row, Col)) {
        continue;
      }
      int Ring = Row;
      int FEN = Col;
      int GraphKey = Ring * 256 + FEN;
      auto qp = Graphs[GraphKey];

      for (int i= 0; i < 256; i++) {
        y0[i] = WThread->Consumer->CDTHistogram[Ring][FEN][0][i];
        y1[i] = WThread->Consumer->CDTHistogram[Ring][FEN][1][i];
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
      if (LogScale) {
        qp->yAxis->setScaleType(QCPAxis::stLogarithmic);
      } else {
        qp->yAxis->setScaleType(QCPAxis::stLinear);
      }
      qp->replot();
    }
  }
}


void CDTGraph::toggle() {
  TogglePlots = (TogglePlots+1)%3;
  updatePlots();
}

void CDTGraph::loglin() {
  LogScale ^= 1;
  updatePlots();
}


void CDTGraph::dead() {

  for (int Row = 0; Row < 11; Row++) {
    for (int Col = 0; Col < 12; Col++) {
      if (ignoreRowCol(Row, Col)) {
        continue;
      }

      int Ring = Row;
      int FEN = Col;
      int DeadCathodes{0};
      int DeadAnodes{0};
      for (int i= 0; i < 256; i++) {
        if (WThread->Consumer->CDTHistogram[Ring][FEN][0][i] == 0) {
          DeadCathodes++;
        }
        if (WThread->Consumer->CDTHistogram[Ring][FEN][1][i] == 0) {
          DeadAnodes++;
        }
      }
      qDebug("Ring %d, FEN %d - dead cathodes %d, dead anodes %d", Ring, FEN, DeadCathodes, DeadAnodes);
    }
  }
}


void CDTGraph::clear() {
  memset(WThread->Consumer->CDTHistogram, 0,
    sizeof(WThread->Consumer->CDTHistogram));
  updatePlots();
}


void CDTGraph::quitProg() {
  QApplication::quit();
}
