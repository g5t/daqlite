// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file GraphBase.h
///
/// \brief Base class for plotting
///
//===----------------------------------------------------------------------===//

#include <QGridLayout>
#include <QPlot/qcustomplot/qcustomplot.h>
#include <QVector>
#include <WorkerThread.h>
#include <map>


#pragma once


class GraphBase : public QObject {
  Q_OBJECT

public:

  /// \brief
  GraphBase(){
    btnToggle = new QPushButton("Select data");
    btnToggleLegend = new QPushButton("legend");
    btnLogLin = new QPushButton("Log/lin");
    btnQuit = new QPushButton("Quit");

    connect(btnToggle, SIGNAL(clicked()), this, SLOT(toggle()));
    connect(btnToggleLegend, SIGNAL(clicked()), this, SLOT(toggleLegend()));
    connect(btnLogLin, SIGNAL(clicked()), this, SLOT(loglin()));
    //connect(btnDead, SIGNAL(clicked()), this, SLOT(dead()));
    //connect(btnClear, SIGNAL(clicked()), this, SLOT(clear()));
    connect(btnQuit, SIGNAL(clicked()), this, SLOT(quitProg()));
  };

  ///\brief
  virtual void setupPlot(QGridLayout * Layout) = 0;

  ///\brief
  virtual void addGraph(QGridLayout * Layout, int Row, int Col) = 0;

  ///\brief
  void addText(QCustomPlot * QCP, std::string Text);

  ///\brief
  void updatePlotPresentation(QCustomPlot * QCP);

  WorkerThread *WThread{nullptr}; // needed to access histogram data
  QPushButton *btnToggle{nullptr};
  QPushButton *btnToggleLegend{nullptr};
  QPushButton *btnLogLin{nullptr};
  QPushButton *btnQuit{nullptr};
  int TogglePlots{0};
  bool LogScale{false};
  bool ToggleLegend{false};
  std::map<int, QCustomPlot *> Graphs;

public Q_SLOTS:

  virtual void updatePlots() = 0;
  virtual void dead() = 0; // deadchannels
  virtual void clear() = 0; // clear histogram data

  void toggle(); // toggle histogram visibility
  void toggleLegend(); // toggle legend visibility
  void loglin(); // toggle log or linear scale
  void quitProg(); // quit

private:

};
