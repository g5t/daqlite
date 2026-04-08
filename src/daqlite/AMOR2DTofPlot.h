// Copyright (C) 2022 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file AMOR2DTofPlot.h
///
/// \brief Creates a 2D TOF plot based on the configuration parameters
//===----------------------------------------------------------------------===//

#pragma once

#include <AbstractPlot.h>

#include <QPlot/qcustomplot/qcustomplot.h>

#include <chrono>
#include <cstdint>
#include <memory>

// Forward declarations
class Configuration;
class ESSConsumer;

class AMOR2DTofPlot : public AbstractPlot {
  Q_OBJECT
public:
  /// \brief plot needs the configurable plotting options
  AMOR2DTofPlot(Configuration &Config, ESSConsumer &Consumer);

  /// \brief adds histogram data, clears periodically then calls
  /// plotDetectorImage()
  void updateData() override;

  /// \brief update plot based on (possibly dynamic) config settings
  void setCustomParameters();

  /// \brief clears histogram data (overridden from AbstractPlot)
  void clearDetectorImage() override;

  /// \brief updates the image
  /// \param Force forces updates of histogram data with zero count
  void plotDetectorImage(bool Force) override;

public slots:
  void showPointToolTip(QMouseEvent *event);

private:
  // QCustomPlot variables
  QCPColorScale *mColorScale{nullptr};
  QCPColorMap *mColorMap{nullptr};
  std::unique_ptr<QCPMarginGroup> mMarginGroup;

  /// \brief allocated according to config in constructor
  static constexpr uint32_t TOF2DX{512};
  static constexpr uint32_t TOF2DY{512};
  uint32_t HistogramData2D[TOF2DX + 1][TOF2DY + 1];

  /// \brief reference time for periodic clearing of histogram
  std::chrono::time_point<std::chrono::high_resolution_clock> t1;
};
