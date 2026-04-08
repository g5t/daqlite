// Copyright (C) 2020 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file TofPlot.cpp
///
//===----------------------------------------------------------------------===//

#include <TofPlot.h>

#include <AbstractPlot.h>
#include <types/PlotType.h>
#include <Configuration.h>
#include <ESSConsumer.h>

#include <QPlot/qcustomplot/qcustomplot.h>
#include <QColor>
#include <QEvent>

#include <algorithm>
#include <string>

using std::string;
using std::vector;

TofPlot::TofPlot(Configuration &Config, ESSConsumer &Consumer)
    : AbstractPlot(PlotType::TOF, Consumer, Config) {
  // Register callback functions for events
  connect(this, &QCustomPlot::mouseMove, this, &TofPlot::showPointToolTip);
  setAttribute(Qt::WA_AlwaysShowToolTips);

  HistogramTofData.resize(mConfig.mTOF.BinSize);

  // This will also allow rescaling the color scale by dragging/zooming
  setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

  axisRect()->setupFullAxesBox(true);

  // Set up the QCPColorMap
  yAxis->setRangeReversed(false);
  yAxis->setSubTicks(true);
  xAxis->setSubTicks(false);
  xAxis->setTickLabelRotation(90);

  mGraph = new QCPGraph(xAxis, yAxis);
  mGraph->setBrush(QBrush(QColor(0, 0, 255, 20)));
  mGraph->setLineStyle(QCPGraph::lsStepCenter);
  mGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

  // We want the color map to have nx * ny data points

  if (mConfig.mPlot.XAxis.empty()) {
    xAxis->setLabel("TOF (μs)");
  } else {
    xAxis->setLabel(mConfig.mPlot.XAxis.c_str());
  }

  yAxis->setLabel("Counts");
  xAxis->setRange(0, 50000);

  setCustomParameters();

}

void TofPlot::setCustomParameters() {
  if (mConfig.mPlot.LogScale) {
    yAxis->setScaleType(QCPAxis::stLogarithmic);
  } else {
    yAxis->setScaleType(QCPAxis::stLinear);
  }
}

void TofPlot::plotDetectorImage(bool Force) {
  setCustomParameters();
  mGraph->data()->clear();
  uint32_t MaxY{0};
  for (size_t i = 0; i < HistogramTofData.size(); i++) {
    if ((HistogramTofData[i] != 0) || (Force)) {
      uint32_t x = i * mConfig.mTOF.MaxValue / mConfig.mTOF.BinSize;
      uint32_t y = HistogramTofData[i];
      if (y > MaxY) {
        MaxY = y;
      }
      mGraph->addData(x, y);
    }
  }

  if (mConfig.mTOF.AutoScaleX) {
    xAxis->setRange(0, mConfig.mTOF.MaxValue * 1.05);
  }
  if (mConfig.mTOF.AutoScaleY) {
    yAxis->setRange(0, MaxY * 1.05);
  }
  replot();
}

void TofPlot::updateData() {
  if (shouldClear()) {
    clearDetectorImage();
  }

  // Get histogram data from Consumer and clear it
  const auto &source = mConfig.mPlot.Source;
  vector<uint32_t> HistogramTof = mConsumer.readData(DataType::HISTOGRAM_TOF, source);

  // Accumulate counts, PixelId 0 does not exist
  for (size_t i = 1; i < HistogramTof.size(); i++) {
    HistogramTofData[i] += HistogramTof[i];
  }
  plotDetectorImage(false);
}

void TofPlot::clearDetectorImage() {
  std::fill(HistogramTofData.begin(), HistogramTofData.end(), 0);
  plotDetectorImage(true);
}

// MouseOver, display coordinate and data in tooltip
void TofPlot::showPointToolTip(QMouseEvent *event) {
  int x = qRound(this->xAxis->pixelToCoord(event->position().x()));

  // Calculate x coord width of the graphical representation of the column
  int xCoordStep = static_cast<int>(mConfig.mTOF.MaxValue / mConfig.mTOF.BinSize);

  // Get the index in data store for the x coordinate
  int xCoordDataIndex = (x - xCoordStep / 2) / xCoordStep;

  // Get column middle TOF value for the x coordinate
  int xCoordTofValue = (x + xCoordStep / 2) / xCoordStep * xCoordStep;

  // Get the count value from the data store
  const bool empty = mGraph->data()->isEmpty();
  double count = (empty) ? 0 : mGraph->data()->at(xCoordDataIndex)->mainValue();

  setToolTip(QString("Tof: %1 Count: %2").arg(xCoordTofValue).arg(count));
}