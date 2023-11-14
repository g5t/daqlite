// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.h
///
/// Main (and only) window for ar51consumer
//===----------------------------------------------------------------------===//

#pragma once

#include <QMainWindow>
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

public slots:

private:

  /// \brief
  WorkerThread *KafkaConsumerThread;

};
