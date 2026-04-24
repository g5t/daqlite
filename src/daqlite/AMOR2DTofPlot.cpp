// Copyright (C) 2020 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file AMOR2DTofPlot.cpp
///
//===----------------------------------------------------------------------===//

#include <AMOR2DTofPlot.h>

#include <AbstractPlot.h>
#include <Configuration.h>
#include <ESSConsumer.h>

#include <types/PlotType.h>
#include <types/Gradients.h>

#include <QEvent>

#include <fmt/format.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

using std::string;
using std::vector;

AMOR2DTofPlot::AMOR2DTofPlot(Configuration &Config,
                             ESSConsumer &Consumer)
    : AbstractPlot(PlotType::TOF2D, Consumer, Config) {
  if ((!(mConfig.mGeometry.YDim <= TOF2DY) ||
       (!(mConfig.mTOF.BinSize <= TOF2DX)))) {
    throw(std::runtime_error("2D TOF histogram size mismatch"));
  }
  memset(HistogramData2D, 0, sizeof(HistogramData2D));

  connect(this, &QCustomPlot::mouseMove, this, &AMOR2DTofPlot::showPointToolTip);
  setAttribute(Qt::WA_AlwaysShowToolTips);

  auto &geom = mConfig.mGeometry;

  // This will also allow rescaling the color scale by dragging/zooming
  setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

  axisRect()->setupFullAxesBox(true);

  // Set up the QCPColorMap
  yAxis->setRangeReversed(true);
  yAxis->setSubTicks(true);
  xAxis->setSubTicks(false);
  xAxis->setTickLabelRotation(90);

  mColorMap = new QCPColorMap(xAxis, yAxis);

  // We want the color map to have nx * ny data points
  const int BinSize = static_cast<int>(mConfig.mTOF.BinSize);
  const int YDim    = static_cast<int>(geom.YDim);
  xAxis->setLabel("TOF");
  yAxis->setLabel("Y");
  mColorMap->data()->setSize(BinSize, YDim);
  mColorMap->data()->setRange(QCPRange(0, mConfig.mTOF.MaxValue),
                              QCPRange(0, YDim)); //

  // Add a color scale
  mColorScale = new QCPColorScale(this);

  // Add it to the right of the main axis rect
  plotLayout()->addElement(0, 1, mColorScale);

  // Scale shall be vertical bar with tick/axis labels
  // right (actually atRight is already the default)
  mColorScale->setType(QCPAxis::atRight);

  // Associate the color map with the color scale
  mColorMap->setColorScale(mColorScale);
  mColorMap->setInterpolate(mConfig.mPlot.Interpolate);
  mColorMap->setTightBoundary(false);
  mColorScale->axis()->setLabel("Counts");

  setCustomParameters();

  // Make sure the axis rect and color scale synchronize their bottom and top
  // margins (so they line up)
  mMarginGroup = std::make_unique<QCPMarginGroup>(this);
  axisRect()->setMarginGroup(QCP::msBottom | QCP::msTop, mMarginGroup.get());
  mColorScale->setMarginGroup(QCP::msBottom | QCP::msTop, mMarginGroup.get());

  // Rescale the key (x) and value (y) axes so the whole color map is visible
  rescaleAxes();

  t1 = std::chrono::high_resolution_clock::now();
}

void AMOR2DTofPlot::setCustomParameters() {
  // Set the color gradient of the color map to one of the presets
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


void AMOR2DTofPlot::clearDetectorImage() {
  memset(HistogramData2D, 0, sizeof(HistogramData2D));
  plotDetectorImage(true);
}

void AMOR2DTofPlot::plotDetectorImage(bool Force) {
  setCustomParameters();

  const int BinSize = static_cast<int>(mConfig.mTOF.BinSize);
  const int YDim    = static_cast<int>(mConfig.mGeometry.YDim);

  for (int y = 0; y < YDim; y++) {
    for (int x = 0; x < BinSize; x++) {
      if ((HistogramData2D[x][y] == 0) && (!Force)) {
        continue;
      }
      mColorMap->data()->setCell(x, y, HistogramData2D[x][y]);
    }
  }

  // Rescale the data dimension (color) such that all data points lie in the
  // span visualized by the color gradient
  mColorMap->rescaleDataRange(true);

  replot();
}

void AMOR2DTofPlot::updateData() {
  // Get newest histogram data from Consumer
  const auto &source = mConfig.mPlot.Source;
  vector<uint32_t> PixelIDs = mConsumer.readData(DataType::PIXEL_ID, source);
  vector<uint32_t> TOFs = mConsumer.readData(DataType::TOF, source);

  // Accumulate counts, PixelId 0 does not exist
  if (PixelIDs.empty()) {
    return;
  }

  const auto XDim = mConfig.mGeometry.XDim;
  // PIXEL_ID and TOF are read in two separate atomic calls; the consumer
  // thread can push more pairs between them, so the returned vectors may
  // differ in size. Iterate over the shorter to keep indices aligned.
  const size_t N = std::min(PixelIDs.size(), TOFs.size());
  for (size_t i = 0; i < N; i++) {
    if (PixelIDs[i] == 0) {
      continue;
    }
    auto tof   = static_cast<size_t>(TOFs[i]);
    auto yvals = static_cast<size_t>((PixelIDs[i] - 1) / XDim);
    HistogramData2D[tof][yvals]++;
  }
  plotDetectorImage(false);
}

// MouseOver
void AMOR2DTofPlot::showPointToolTip(QMouseEvent *event) {
  int x = qRound(this->xAxis->pixelToCoord(event->position().x()));
  int y = qRound(this->yAxis->pixelToCoord(event->position().y()));

  setToolTip(QString("%1 , %2").arg(x).arg(y));
}
