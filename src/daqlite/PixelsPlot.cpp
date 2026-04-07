// Copyright (C) 2020 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file PixelsPlot.cpp
///
//===----------------------------------------------------------------------===//

#include <PixelsPlot.h>

#include <AbstractPlot.h>
#include <Configuration.h>
#include <ESSConsumer.h>

#include <logical_geometry/ESSGeometry.h>
#include <types/Gradients.h>
#include <types/PlotType.h>

#include <fmt/format.h>

#include <algorithm>
#include <chrono>
#include <ratio>
#include <string>

using std::string;
using std::vector;

// clang-format off
PixelsPlot::PixelsPlot(Configuration &Config, ESSConsumer &Consumer,
                       Projection Proj)
    : AbstractPlot(PlotType::PIXELS, Consumer, Config)
    , mProjection(Proj) {
// clang-format on

// Register callback functions for events
  connect(this, &QCustomPlot::mouseMove, this, &PixelsPlot::showPointToolTip);
  setAttribute(Qt::WA_AlwaysShowToolTips);

  auto &geom = mConfig.mGeometry;
  LogicalGeometry = new ESSGeometry(geom.XDim, geom.YDim, geom.ZDim, 1);
  HistogramData.resize(LogicalGeometry->max_pixel() + 1);

  // this will also allow rescaling the color scale by dragging/zooming
  setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

  axisRect()->setupFullAxesBox(true);

  // set up the QCPColorMap:
  yAxis->setRangeReversed(true);
  yAxis->setSubTicks(true);
  xAxis->setSubTicks(false);
  xAxis->setTickLabelRotation(90);

  mColorMap = new QCPColorMap(xAxis, yAxis);

  // we want the color map to have nx * ny data points
  const int XDim = static_cast<int>(geom.XDim);
  const int YDim = static_cast<int>(geom.YDim);
  const int ZDim = static_cast<int>(geom.ZDim);
  if (mProjection == ProjectionXY) {
    xAxis->setLabel("X");
    yAxis->setLabel("Y");
    mColorMap->data()->setSize(XDim, YDim);
    mColorMap->data()->setRange(QCPRange(0, XDim - 1),
                                QCPRange(0, YDim - 1)); //
  } else if (mProjection == ProjectionXZ) {
    xAxis->setLabel("X");
    yAxis->setLabel("Z");
    mColorMap->data()->setSize(XDim, ZDim);
    mColorMap->data()->setRange(QCPRange(0, XDim - 1),
                                QCPRange(0, ZDim - 1));
  } else {
    xAxis->setLabel("Y");
    yAxis->setLabel("Z");
    mColorMap->data()->setSize(YDim, ZDim);

    mColorMap->data()->setRange(QCPRange(0, YDim - 1),
                                QCPRange(0, ZDim - 1));
  }
  // add a color scale:
  mColorScale = new QCPColorScale(this);

  // add it to the right of the main axis rect
  plotLayout()->addElement(0, 1, mColorScale);

  // scale shall be vertical bar with tick/axis labels
  // right (actually atRight is already the default)
  mColorScale->setType(QCPAxis::atRight);

  // associate the color map with the color scale
  mColorMap->setColorScale(mColorScale);
  mColorMap->setInterpolate(mConfig.mPlot.Interpolate);
  mColorMap->setTightBoundary(false);
  mColorScale->axis()->setLabel("Counts");

  setCustomParameters();

  // make sure the axis rect and color scale synchronize their bottom and top
  // margins (so they line up):
  QCPMarginGroup *marginGroup = new QCPMarginGroup(this);
  axisRect()->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);
  mColorScale->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);

  // rescale the key (x) and value (y) axes so the whole color map is visible:
  rescaleAxes();

  t1 = std::chrono::high_resolution_clock::now();
}

void PixelsPlot::setCustomParameters() {
  // set the color gradient of the color map to one of the presets:
  QCPColorGradient Gradient(getColorGradient(mConfig.mPlot.ColorGradient));

  if (mConfig.mPlot.InvertGradient) {
    Gradient = Gradient.inverted();
  }

  mColorMap->setGradient(Gradient);
  if (mConfig.mPlot.LogScale) {
    mColorMap->setDataScaleType(QCPAxis::stLogarithmic);
  } else {
    mColorMap->setDataScaleType(QCPAxis::stLinear);
  }
}


void PixelsPlot::clearDetectorImage() {
  std::fill(HistogramData.begin(), HistogramData.end(), 0);
  plotDetectorImage(true);
}

void PixelsPlot::plotDetectorImage(bool Force) {
  setCustomParameters();

  // if scales match the dimensions (xdim 400, range 0, 399) then cell indexes
  // and coordinates match. PixelId 0 does not exist.
  for (size_t i = 1; i < HistogramData.size(); i++) {
    if ((HistogramData[i] != 0) || (Force)) {
      int xIndex = static_cast<int>(LogicalGeometry->x(i));
      int yIndex = static_cast<int>(LogicalGeometry->y(i));
      int zIndex = static_cast<int>(LogicalGeometry->z(i));

      // here we could
      // x, y, z = pos(i)

      if (mProjection == ProjectionXY) {
        // printf("XY: x,y,z %d, %d, %d: count %d\n", xIndex, yIndex, zIndex,
        // HistogramData[i]);
        mColorMap->data()->setCell(xIndex, yIndex, HistogramData[i]);
      } else if (mProjection == ProjectionXZ) {
        // printf("XZ: x,y,z %d, %d, %d: count %d\n", xIndex, yIndex, zIndex,
        // HistogramData[i]);
        mColorMap->data()->setCell(xIndex, zIndex, HistogramData[i]);
      } else {
        // printf("YZ: x,y,z %d, %d, %d: count %d\n", xIndex, yIndex, zIndex,
        // HistogramData[i]);
        mColorMap->data()->setCell(yIndex, zIndex, HistogramData[i]);
      }
    }
  }

  // rescale the data dimension (color) such that all data points lie in the
  // span visualized by the color gradient:
  mColorMap->rescaleDataRange(true);

  replot();
}

void PixelsPlot::updateData() {
  auto t2 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<int64_t, std::nano> elapsed = t2 - t1;

  // update histogram data from Consumer according to the source specified in
  // the config
  const auto &source = mConfig.mPlot.Source;
  vector<uint32_t> Histogram = mConsumer.readData(DataType::HISTOGRAM, source);

  int64_t nsBetweenClear = 1000000000LL * mConfig.mPlot.ClearEverySeconds;
  if (mConfig.mPlot.ClearPeriodic && (elapsed.count() >= nsBetweenClear)) {
    t1 = std::chrono::high_resolution_clock::now();
    std::fill(HistogramData.begin(), HistogramData.end(), 0);

    // Periodically clear the histogram
    plotDetectorImage(true);
  }

  // Accumulate counts, PixelId 0 does not exist
  for (size_t i = 1; i < Histogram.size(); i++) {
    HistogramData[i] += Histogram[i];
  }
  plotDetectorImage(false);
}

// MouseOver, display coordinate and data in tooltip
void PixelsPlot::showPointToolTip(QMouseEvent *event) {
  int x = qRound(this->xAxis->pixelToCoord(event->position().x()));
  int y = qRound(this->yAxis->pixelToCoord(event->position().y()));

  double count = mColorMap->data()->data(x, y);

  setToolTip(QString("X: %1 , Y: %2, Count: %3").arg(x).arg(y).arg(count));
}
