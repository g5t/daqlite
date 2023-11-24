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
#include <QPlot/qcustomplot/qcustomplot.h>
#include <QPlot/QPlot.h>
#include <QVector>
#include <WorkerThread.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(std::string Broker, std::string Topic, QWidget *parent = nullptr);
  ~MainWindow();

  /// \brief spin up a thread for consuming topic
  void startKafkaConsumerThread(std::string Broker, std::string Topic);

  /// \brief intial setup
  void setupPlottingWidgets(int Row, int Col);

  /// \brief Use keyboard shortcuts to affect plotting
  void keyPressEvent(QKeyEvent *event);

public slots:

  void updatePlots();
  void toggle(); // toggle histogram visibility
  void clear(); // clear histogram data
  void quitProg();

private:
  /// \brief
  WorkerThread *KafkaConsumerThread;

  QGridLayout *layout{nullptr};
  QVector<double> x, y, y2;
  QCustomPlot * Graphs[5][3];
  int TogglePlots{0};
};
