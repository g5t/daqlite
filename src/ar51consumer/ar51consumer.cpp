// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file ar52consumer.cpp
///
/// \brief ar52 schema consumer - proof of concept
///
/// Identifies types of modes
//===----------------------------------------------------------------------===//

#include <ESSConsumer.h>
#include <MainWindow.h>
#include <QApplication>
#include <QCommandLineParser>
#include <WorkerThread.h>
#include <stdio.h>


int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  QCommandLineParser CLI;
  CLI.setApplicationDescription("Daquiri light - when you're driving home");
  CLI.addHelpOption();

  QCommandLineOption kafkaBrokerOption("b", "Kafka broker", "unusedDefault");
  CLI.addOption(kafkaBrokerOption);
  QCommandLineOption kafkaTopicOption("t", "Kafka topic", "unusedDefault");
  CLI.addOption(kafkaTopicOption);

  CLI.process(app);

  MainWindow win {
    CLI.value(kafkaBrokerOption).toStdString(),
    CLI.value(kafkaTopicOption).toStdString()
  };
  win.resize(1400, 600);
  return app.exec();
}
