// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file VMM3aGraph
///
/// \brief Class for handling plotting of VMM readouts
///
//===----------------------------------------------------------------------===//

#include <VMM3aGraph.h>
#include <fmt/format.h>


bool VMM3aGraph::ignoreEntry(int Ring, int Hybrid) {
  if ((Hybrid > 4) or (Ring > 4)) {
    return true;
  }
  return false;
}


void VMM3aGraph::setupPlot(QGridLayout * Layout) {

  for (int i = 0; i < NumChannels; i++) {
    x.push_back(i);
    y0.push_back(0);
    y1.push_back(0);
  }

  for (int Ring = 0; Ring < 4; Ring++) {
    for (int Hybrid = 0; Hybrid < 5; Hybrid++) {
      if (ignoreEntry(Ring, Hybrid)) {
        continue;
      }
      addGraph(Layout, Ring, Hybrid);
    }
  }

  // Add final row of buttons (some are defined in GraphBase.h)
  QPushButton *btnDead = new QPushButton("Dead");
  QPushButton *btnClear = new QPushButton("Clear");

  QHBoxLayout *hblayout = new QHBoxLayout();
  hblayout->addWidget(btnToggle);
  hblayout->addWidget(btnToggleLegend);
  hblayout->addWidget(btnLogLin);
  QHBoxLayout *hblayout2 = new QHBoxLayout();
  hblayout2->addWidget(btnDead);
  hblayout2->addWidget(btnClear);
  hblayout2->addWidget(btnQuit);

  Layout->addLayout(hblayout, 4, 0);
  Layout->addLayout(hblayout2, 4, 1);

  connect(btnDead, SIGNAL(clicked()), this, SLOT(dead()));
  connect(btnClear, SIGNAL(clicked()), this, SLOT(clear()));

  /// Update timer
  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &VMM3aGraph::updatePlots);
  timer->start(1000);
}

  /// Create graph(s) in the specified grid layout
  ///  allocate new QCustomPlot
  ///  set plot text
  ///  set x-axis range (permanent)
  ///  set y-axis range (intial)
void VMM3aGraph::addGraph(QGridLayout * Layout, int Ring, int Hybrid) {
  QCustomPlot * QCP = new QCustomPlot();
  int GraphKey = Ring * 256 + Hybrid;
  Graphs[GraphKey] = QCP;

  addText(QCP, fmt::format("R{}/H{}", Ring, Hybrid));
  QCP->legend->setBorderPen(QPen(Qt::transparent));
  if (ToggleLegend) {
    QCP->legend->setVisible(true);
  }
  QCP->xAxis->setRange(0, 63);
  QCP->yAxis->setRange(0, 5);
  QCP->addGraph();
  QCP->graph(0)->setName("strips");
  QCP->graph(0)->setData(x, y0);
  QCP->graph(0)->setLineStyle(QCPGraph::LineStyle::lsStepLeft);
  QCP->graph(0)->setBrush(QBrush(QColor(20,50,255,20)));
  QCP->graph(0)->setPen(QPen(QColor(0, 0, 255), 0));

  QCP->addGraph();
  QCP->graph(1)->setName("wires");
  QCP->graph(1)->setData(x, y1);
  QCP->graph(1)->setLineStyle(QCPGraph::LineStyle::lsStepLeft);
  QCP->graph(1)->setBrush(QBrush(QColor(255,50,20,20)));
  QCP->graph(1)->setPen(QPen(QColor(190, 50, 20), 0));
  Layout->addWidget(QCP, Ring, Hybrid);
}


/// Update plot: get new data, toggle log/lin scale,
/// toggle histograms first/second/both, rescale
void VMM3aGraph::updatePlots() {
  for (int Ring = 0; Ring < 4; Ring++) {
    for (int Hybrid = 0; Hybrid < 5; Hybrid++) {
      if (ignoreEntry(Ring, Hybrid)) {
        continue;
      }
      int GraphKey = Ring * 256 + Hybrid;
      auto qp = Graphs[GraphKey];

      for (int i= 0; i < NumChannels; i++) {
        y0[i] = WThread->Consumer->Histogram[Ring][Hybrid][0][i]; // strips
        y1[i] = WThread->Consumer->Histogram[Ring][Hybrid][1][i]; // wires
      }

      qp->graph(0)->setData(x, y0);
      qp->graph(1)->setData(x, y1);

      updatePlotPresentation(qp);
      qp->replot();
    }
  }
}


/// Count 'dead' channels - or rather count number of channels with zero count
void VMM3aGraph::dead() {
  for (int Ring = 0; Ring < 3; Ring++) {
    for (int Hybrid = 0; Hybrid < 5; Hybrid++) {
      if (ignoreEntry(Ring, Hybrid)) {
        continue;
      }
      int DeadAsic0{0};
      int DeadAsic1{0};
      for (int i= 0; i < NumChannels; i++) {
        if (WThread->Consumer->Histogram[Ring][Hybrid][0][i] == 0) {
          DeadAsic0++;
        }
        if (i >=16 and i <= 47 and WThread->Consumer->Histogram[Ring][Hybrid][1][i] == 0) {
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
