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
  textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
  textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
  textLabel->position->setCoords(0.9, 0); // top right
  textLabel->setText(QString(Text.c_str()));
  textLabel->setFont(QFont("Helvetica", 10, QFont::Normal)); // or Bold
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
