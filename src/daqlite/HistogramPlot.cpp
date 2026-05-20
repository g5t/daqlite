// Copyright (C) 2020 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file HistogramPlot.cpp
///
//===----------------------------------------------------------------------===//

#include <HistogramPlot.h>

#include <AbstractPlot.h>
#include <types/PlotType.h>
#include <Configuration.h>
#include <ESSConsumer.h>

#include <QPlot/qcustomplot/qcustomplot.h>
#include <QBrush>
#include <QColor>
#include <QEvent>

#include <fmt/format.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

using std::string;
using std::vector;

HistogramPlot::HistogramPlot(Configuration &Config, ESSConsumer &Consumer)
    : AbstractPlot(PlotType::HISTOGRAM, Consumer, Config) {
  // Register callback functions for events
  connect(this, &QCustomPlot::mouseMove, this, &HistogramPlot::showPointToolTip);
  setAttribute(Qt::WA_AlwaysShowToolTips);

  HistogramYAxisValues.resize(mConfig.mTOF.BinSize);

  // This will also allow rescaling the axes by dragging/zooming
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

  yAxis->setLabel("Value Sum");
  xAxis->setRange(0, 50000);

  setCustomParameters();

}

void HistogramPlot::setCustomParameters() {
  if (mConfig.mPlot.LogScale) {
    yAxis->setScaleType(QCPAxis::stLogarithmic);
  } else {
    yAxis->setScaleType(QCPAxis::stLinear);
  }
}

void HistogramPlot::plotDetectorImage(bool) {
  setCustomParameters();
  mGraph->data()->clear();

  for (size_t i = 0; i < HistogramYAxisValues.size(); i++) {
    // Calculate the middle x value of the bin to place the data point
    auto binWidth = HistogramXAxisValues[i + 1] - HistogramXAxisValues[i];
    auto middleXValue = HistogramXAxisValues[i] + binWidth / 2.0;

    double ScaledXValue = middleXValue / mConfig.mTOF.Scale;

    mGraph->addData(ScaledXValue, HistogramYAxisValues[i]);
  }

  if (mConfig.mTOF.AutoScaleX && !HistogramXAxisValues.empty()) {
    double MinX = HistogramXAxisValues.front();
    double MaxX = HistogramXAxisValues.back();
    xAxis->setRange(MinX / mConfig.mTOF.Scale, MaxX / mConfig.mTOF.Scale * 1.05);
  }
  if (mConfig.mTOF.AutoScaleY) {
    yAxis->setRange(0, mMaxY * 1.05);
  }

  replot();
}

void HistogramPlot::updateData() {
  if (shouldClear()) {
    clearDetectorImage();
  }

  // Continue the update only if we have data available from the consumer
  const auto &source = mConfig.mPlot.Source;
  if (mConsumer.getDataSize(DataType::HISTOGRAM, source) == 0 || mConsumer.getDataSize(DataType::TOF, source) == 0) {
    return;
  }

  vector<uint32_t> YAxisValues = mConsumer.readData(DataType::HISTOGRAM, source);
  auto TofValues = mConsumer.readData(DataType::TOF, source, false);

  HistogramXAxisValues = TofValues;
  if (YAxisValues.size() != HistogramXAxisValues.size() - 1) {
    fmt::print("HistogramPlot::updateData() - Y axis values do not match x "
               "axis values. Skip processing!\n");
    return;
  }

  if (HistogramYAxisValues.size() < YAxisValues.size()) {
    HistogramYAxisValues.resize(YAxisValues.size());
  }

  for (size_t i = 0; i < YAxisValues.size(); i++) {
    HistogramYAxisValues[i] += YAxisValues[i];
    mMaxY = std::max(mMaxY, HistogramYAxisValues[i]);
  }

  plotDetectorImage(false);
}

void HistogramPlot::clearDetectorImage() {
  std::fill(HistogramYAxisValues.begin(), HistogramYAxisValues.end(), 0);
  mMaxY = 0;
  plotDetectorImage(true);
}

// MouseOver, display coordinate and data in tooltip
void HistogramPlot::showPointToolTip(QMouseEvent *event) {
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

  setToolTip(QString("Tof: %1 Value: %2").arg(xCoordTofValue).arg(count));
}