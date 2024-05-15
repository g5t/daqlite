// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file GraphBase.cpp
///
/// \brief Base Class for plotting
///
//===----------------------------------------------------------------------===//

#include <GraphBase.h>

void GraphBase::addText(QCustomPlot * QCP, std::string Text) {
  QCPItemText *textLabel = new QCPItemText(QCP);
  textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignRight);
  textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
  textLabel->position->setCoords(0.99, 0); // top right
  textLabel->setText(QString(Text.c_str()));
  textLabel->setFont(QFont("Helvetica", 10, QFont::Normal)); // or Bold
}


void GraphBase::updatePlotPresentation(QCustomPlot * QCP) {
  QCP->graph(0)->setVisible(false);
  QCP->graph(1)->setVisible(false);
  if (TogglePlots == 0 or TogglePlots == 1) {
    QCP->graph(0)->setVisible(true);
  }
  if (TogglePlots == 0 or TogglePlots == 2) {
    QCP->graph(1)->setVisible(true);
  }

  if (ToggleLegend) {
    QCP->legend->setVisible(true);
  } else {
    QCP->legend->setVisible(false);
  }

  QCP->yAxis->rescale();
  if (LogScale) {
    QCP->yAxis->setScaleType(QCPAxis::stLogarithmic);
  } else {
    QCP->yAxis->setScaleType(QCPAxis::stLinear);
  }
}


void GraphBase::toggle() {
  TogglePlots = (TogglePlots+1)%3;
  updatePlots();
}

void GraphBase::toggleLegend() {
  ToggleLegend ^= 1;
  updatePlots();
}

void GraphBase::loglin() {
  LogScale ^= 1;
  updatePlots();
}

void GraphBase::quitProg() {
  QApplication::quit();
}
