// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file VMM3aGraph
///
/// \brief Class for handling plotting of VMM readouts
///
//===----------------------------------------------------------------------===//

#include <QGridLayout>
#include <QPlot/qcustomplot/qcustomplot.h>
#include <QVector>
#include <WorkerThread.h>
#include <map>


#pragma once


class VMM3aGraph : public QObject {
  Q_OBJECT

public:

  /// \brief
  VMM3aGraph(){};

  ///\brief
  void setupPlot(QGridLayout * Layout);

  /// \brief
  void addGraph(QGridLayout * Layout, int Row, int Col);

  WorkerThread *WThread{nullptr}; // needed to access histogram data

public Q_SLOTS:

  void updatePlots();
  void toggle(); // toggle histogram visibility
  void dead(); // deadchannels
  void clear(); // clear histogram data
  void quitProg(); // quit

private:
  std::map<int, QCustomPlot *> Graphs;
  QVector<double> x, y0, y1;
  int TogglePlots{0};
  int FindDead{0};
};