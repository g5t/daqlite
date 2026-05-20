// Copyright (C) 2020 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file daqlite.cpp
///
/// \brief Daquiri Light main application
///
/// Handles command line option(s), instantiates GUI
//===----------------------------------------------------------------------===//

#include <Configuration.h>
#include <DaqliteMsgFilter.h>
#include <MainWindow.h>
#include <WorkerThread.h>

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QPushButton>
#include <QString>
#include <QStringList>

#include <fmt/format.h>

#include <memory>
#include <string>
#include <vector>

using std::string;
using std::vector;

namespace {
  /// \brief Extract specified command line options and add them to the plot configuration

  /// \param CLI     Command line options
  /// \param Config  Plot configuration
  void setKafkaOptions(const QCommandLineParser &CLI, Configuration &Config) {
    for (const QString &option: CLI.optionNames()) {
      if (option == "b") {
        Config.mKafka.Broker = CLI.value(option).toStdString();
      }

      else if (option == "t") {
        Config.mKafka.Topic = CLI.value(option).toStdString();
      }

      else if (option == "k") {
        Config.mKafkaConfigFile = CLI.value(option).toStdString();
      }
    }
  }

  /// \brief Print a one-shot summary of the resolved setup
  void printSetupSummary(const string &PlotConfigFile,
                        const vector<Configuration> &Configs) {
    const Configuration &Main = Configs.front();
    fmt::print("\n=== daqlite setup ===\n");
    fmt::print("  Plot config  : {}\n", PlotConfigFile);
    fmt::print("  Kafka broker : {}\n", Main.mKafka.Broker);
    fmt::print("  Kafka topic  : {}\n", Main.mKafka.Topic);
    fmt::print("  Kafka config : {}\n", Main.mKafkaConfigFile);
    fmt::print("  Plots        : {} window(s)\n", Configs.size());
    for (const auto &Config: Configs) {
      fmt::print("    - {}\n", Config.mPlot.WindowTitle);
    }
    fmt::print("=====================\n");
  }
}

int main(int argc, char *argv[]) {
  installDaqliteMsgFilter();
  QApplication app(argc, argv);

  // Handle all command line args
  QCommandLineParser CLI;
  CLI.setApplicationDescription(
      "Daquiri light - Qt visualizer for ESS detector data streamed via Kafka.\n"
      "(When you're driving home.)");
  CLI.addHelpOption();

  // Add specified options. Empty valueName means the option is a flag (no value).
  vector<std::tuple<QStringList, QString, QString>> Options = {
    {{"f", "config"},       "Configuration file.",                          "file"},
    {{"b", "broker"},       "Kafka broker.",                                "host:port"},
    {{"t", "topic"},        "Kafka topic.",                                 "topic"},
    {{"k", "kafka-config"}, "Kafka configuration file.",                    "kafka-config"},
    {{"q", "quiet"},        "Suppress setup summary output.",               ""},
    {{"d", "debug"},        "Enable verbose configuration parsing output.", ""},
  };
  for (const auto& [names, info, valueName]: Options) {
    QCommandLineOption option(names, info, valueName);
    CLI.addOption(option);
  }
  CLI.process(app);

  // Validate required args and propagate global flags before any config is loaded
  if (!CLI.isSet("f")) {
    fmt::print(stderr,
               "Error: missing required option -f/--config <file>.\n"
               "Run with --help for usage.\n");
    return 1;
  }
  // Must be set before getConfigurations() so JSON parsing honours the flag
  Configuration::sDebug = CLI.isSet("d");

  // Parent button used to quit all plot widgets
  QPushButton QuitButton("&Quit");
  QObject::connect(&QuitButton, &QPushButton::clicked, &app, &QApplication::quit);

  // ---------------------------------------------------------------------------
  // Load configurations and apply CLI overrides
  const string FileName = CLI.value("f").toStdString();
  vector<Configuration> confs = Configuration::getConfigurations(FileName);
  for (auto &Config: confs) {
    setKafkaOptions(CLI, Config);
  }

  if (!CLI.isSet("q")) {
    printSetupSummary(FileName, confs);
  }

  // Setup worker thread from the (now-overridden) primary configuration
  std::shared_ptr<WorkerThread> Worker = std::make_shared<WorkerThread>(confs.front());

  // Setup a window for each plot
  for (const auto &Config: confs) {
    MainWindow* w = new MainWindow(Config, Worker.get());
    w->setWindowTitle(QString::fromStdString(Config.mPlot.WindowTitle));
    w->setParent(&QuitButton, Qt::Window);
    w->show();
  }

  // Start the worker and let the Qt event handler take over
  Worker->start();

  return app.exec();
}
