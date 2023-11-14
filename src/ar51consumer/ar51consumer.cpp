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
#include <QGridLayout>
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

  MainWindow win{
    CLI.value(kafkaBrokerOption).toStdString(),
    CLI.value(kafkaTopicOption).toStdString()
  };


  QGridLayout *layout = new QGridLayout;
  //creating a QWidget, and setting the WCwindow as parent
  QWidget * widget = new QWidget();

  //set the gridlayout for the widget
  widget->setLayout(layout);

  //setting the WCwindow's central widget
  win.setCentralWidget(widget);
  win.show();
  return app.exec();
}
