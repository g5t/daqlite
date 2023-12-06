// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.h
///
/// Main (and only) window for ar51consumer
//===----------------------------------------------------------------------===//

#pragma once

#include <QGridLayout>
#include <QMainWindow>
#include <QPlot/QPlot.h>
//#include <CAENGraph.h>
#include <CDTGraph.h>
#include <VMM3aGraph.h>
#include <WorkerThread.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(std::string Broker, std::string Topic, std::string Readout,
    QWidget *parent = nullptr);
  ~MainWindow();

  /// \brief spin up a thread for consuming topic
  void startConsumer(std::string Broker, std::string Topic);

  /// \brief intial setup
  void setupPlottingWidgets(int Row, int Col);

  /// \brief Use keyboard shortcuts to affect plotting
  //void keyPressEvent(QKeyEvent *event);

  void quitProg();

private:
  /// \brief
  WorkerThread *Consumer;

  QGridLayout layout;

  VMM3aGraph vmmgraph{};
  CDTGraph cdtgraph{};
  //CAENGraph caengraph{};
};
