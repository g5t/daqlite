// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file VMM3aGraph
///
/// \brief Class for handling plotting of VMM readouts
///
//===----------------------------------------------------------------------===//

#include <GraphBase.h>


#pragma once


class VMM3aGraph : public GraphBase {

public:

  /// \brief
  VMM3aGraph(){};

  ///\brief
  void setupPlot(QGridLayout * Layout);

  ///\brief
  void addGraph(QGridLayout * Layout, int Row, int Col);

  ///\brief helper function for irregular layouts
  bool ignoreEntry(int Ring, int Hybrid);


  WorkerThread *WThread{nullptr}; // needed to access histogram data

public Q_SLOTS:

  void updatePlots();
  void dead(); // deadchannels
  void clear(); // clear histogram data

private:
  QVector<double> x, y0, y1;
  int NumChannels{64};
};
