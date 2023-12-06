// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file CDTraph
///
/// \brief Class for handling plotting of CDT readouts
///
//===----------------------------------------------------------------------===//

#include <GraphBase.h>

#pragma once


class CAENGraph :public GraphBase {
  Q_OBJECT

public:

  ///\brief
  CAENGraph(){};

  ///\brief
  void setupPlot(QGridLayout * Layout);

  ///\brief
  void addGraph(QGridLayout * Layout, int Row, int Col);

  ///\brief
  bool ignoreEntry(int Ring, int FEN);

  WorkerThread *WThread{nullptr}; // needed to access histogram data

  //QCPColorMap * mColorMap{nullptr};
  std::map<int, QCPColorMap *> CMGraphs;

public Q_SLOTS:

  void updatePlots();
  void dead(); // deadchannels
  void clear(); // clear histogram data

private:
  QVector<double> x, y0, y1;
  int NumChannels{256};
};
