// Copyright (C) 2020 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file PixelsPlot.h
///
/// \brief Creates a QCustomPlot based on the configuration parameters
//===----------------------------------------------------------------------===//

#pragma once

#include <AbstractPlot.h>

#include <QPlot/qcustomplot/qcustomplot.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

// Forward declarations
class Configuration;
class ESSConsumer;
class ESSGeometry;

class PixelsPlot : public AbstractPlot {
  Q_OBJECT
public:
  enum Projection {ProjectionXY, ProjectionXZ, ProjectionYZ};

  /// \brief plot needs the configurable plotting options
  PixelsPlot(Configuration &Config, ESSConsumer&, Projection Proj);

  /// \brief Destructor must be defined in .cpp due to forward declaration of ESSGeometry.
  ///        This is required by std::unique_ptr with a forward-declared type
  ~PixelsPlot();

  /// \brief adds histogram data, clears periodically then calls
  /// plotDetectorImage()
  void updateData() override;

  /// \brief update plot based on (possibly dynamic) config settings
  void setCustomParameters();

  /// \brief clears histogram data
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

  std::vector<uint32_t> HistogramData;

  /// \brief for calculating x, y, z from pixelid
  std::unique_ptr<ESSGeometry> LogicalGeometry;

  //
  Projection mProjection;

  /// \brief reference time for periodic clearing of histogram
  std::chrono::time_point<std::chrono::high_resolution_clock> t1;
};
