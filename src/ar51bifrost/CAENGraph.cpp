// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file CAENGraph
///
/// \brief Class for handling plotting of CDT readouts
///
//===----------------------------------------------------------------------===//

#include <CAENGraph.h>
#include <fmt/format.h>
#include "bifrost.h"


bool CAENGraph::ignoreEntry(int arc, int triplet) {
  if (arc < 0 || arc >= bifrostHistograms::ARCS) return true;
  if (triplet < 0 || triplet >= bifrostHistograms::TRIPLETS) return true;
  return false;
}


void CAENGraph::setupPlot(QGridLayout * Layout) {
  printf("setup plot\n");
  for (int i = 0; i < NumChannels; i++) {
    x.push_back(i);
    y0.push_back(0);
    y1.push_back(0);
  }

  for (int arc = 0; arc < bifrostHistograms::ARCS; ++arc){
    for (int triplet = 0; triplet < bifrostHistograms::TRIPLETS; ++triplet){
      if (ignoreEntry(arc, triplet)) {
        continue;
      }
      addGraph(Layout, arc, triplet);
    }
  }

  // Add final row of buttons (some inherited from base)
  QPushButton *btnDead = new QPushButton("Dead");
  QPushButton *btnClear = new QPushButton("Clear");

  QHBoxLayout *hblayout = new QHBoxLayout();
  //hblayout->addWidget(btnToggle);
  //hblayout->addWidget(btnToggleLegend);
  //hblayout->addWidget(btnLogLin);
  Layout->addLayout(hblayout, 11, 0);

  QHBoxLayout *hblayout2 = new QHBoxLayout();
  //hblayout2->addWidget(btnDead);
  hblayout2->addWidget(btnClear);
  hblayout2->addWidget(btnQuit);
  Layout->addLayout(hblayout2, 11, 1);

  connect(btnDead, SIGNAL(clicked()), this, SLOT(dead()));
  connect(btnClear, SIGNAL(clicked()), this, SLOT(clear()));


  /// Update timer
  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &CAENGraph::updatePlots);
  timer->start(1000);
}


  /// \brief
void CAENGraph::addGraph(QGridLayout * Layout, int module, int triplet) {
  auto qcp = new QCustomPlot();
  int GraphKey = module * 256 + triplet;
  Graphs[GraphKey] = qcp;

  Layout->addWidget(qcp, module, triplet);

  //
  auto * qcm = new QCPColorMap(qcp->xAxis, qcp->yAxis);
  CMGraphs[GraphKey] = qcm;

  qcm->data()->setSize(xDim, yDim);
  qcm->data()->setRange(QCPRange(0, bifrostHistograms::BIN2D-1), QCPRange(0, bifrostHistograms::BIN2D-1));

  double x, y, z;
  for (int xIndex=0; xIndex<xDim; ++xIndex)
  {
    for (int yIndex=0; yIndex<yDim; ++yIndex)
    {
      qcm->data()->cellToCoord(xIndex, yIndex, &x, &y);
      double r = 3*qSqrt(x*x+y*y)+1e-2;
      z = 2*x*(qCos(r+2)/r-qSin(r+2)/r); // the B field strength of dipole radiation (modulo physical constants)
      qcm->data()->setCell(xIndex, yIndex, z);
    }
  }

  auto *colorScale = new QCPColorScale(qcp);
  //qcp->plotLayout()->addElement(0, 1, colorScale);
  //colorScale->setType(QCPAxis::atRight);
  qcm->setColorScale(colorScale);
  qcm->setGradient(QCPColorGradient::gpPolar);
  qcm->rescaleDataRange();
  qcp->rescaleAxes();

  addText(qcp, fmt::format("M{}/T{}", module, triplet));
}


void CAENGraph::updatePlots() {
  //printf("update plot\n");
  for (int Ring = 0; Ring < 4; Ring++) {
    for (int FEN = 0; FEN < 4; FEN++) {
      if (ignoreEntry(Ring, FEN)) {
        continue;
      }
      int GraphKey = Ring * 256 + FEN;
      auto qcp = Graphs[GraphKey];
      auto qcm = CMGraphs[GraphKey];

      // for (int i= 0; i < NumChannels; i++) {
      //   y0[i] = WThread->Consumer->CDTHistogram[Ring][FEN][0][i];
      //   y1[i] = WThread->Consumer->CDTHistogram[Ring][FEN][1][i];
      // }


      phase += 1;

      double x, y, z;
      for (int xIndex=0; xIndex<xDim; ++xIndex)
      {
        for (int yIndex=0; yIndex<yDim; ++yIndex)
        {
          qcm->data()->cellToCoord(xIndex, yIndex, &x, &y);
          double r = 3*qSqrt(x*x+y*y)+1e-2;
          z = 2*x*(qCos(r+2+ phase/10.0)/r-qSin(r+2)/r); // the B field strength of dipole radiation (modulo physical constants)
          qcm->data()->setCell(xIndex, yIndex, z);
        }
      }

      //updatePlotPresentation(qp);

      qcp->replot();
    }
  }
}


///\brief Button signals below
void CAENGraph::dead() {

  for (int arc = 0; arc < bifrostHistograms::ARCS; ++arc){
    for (int triplet = 0; triplet < bifrostHistograms::TRIPLETS; ++triplet){
      if (ignoreEntry(arc, triplet)) {
        continue;
      }
      if (WThread->Consumer->histograms.is_empty(arc, triplet)) {
        qDebug("Arc %d, Triplet %d - dead", arc, triplet);
      }
    }
  }

}


void CAENGraph::clear() {
  WThread->Consumer->histograms.reset();
  updatePlots();
}
