#include <iostream>
#include <map>
#include <QApplication>
#include <QCommandLineParser>
#include <fmt/format.h>

#include "Configuration.h"
#include "Calibration.h"
#include "Time.h"
#include "WorkerThread.h"
#include "DataManager.h"

void do_work(Configuration & configuration, Calibration & calibration, std::time_t from, std::time_t to, const std::string& output_file) {
  auto tubes = configuration.Instrument.units_per_group;
  auto pixelation = configuration.Instrument.pixels_per_unit;
  auto data = new ::bifrost::data::Manager(5, 9, tubes, pixelation, calibration);
  auto worker = new WorkerThread(data, configuration);
  // setting the time range does not work until the consumer is running due to how librdkafka seeks/assigns the partition
  // consume messages
  worker->start();
  // now we can update the time range
  worker->consume_from(from);
  worker->consume_until(to);
  // wait for worker to finish
  worker->wait();
  // store the data
  data->save_to(output_file);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    auto now = std::time({});
    char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
    std::strftime(std::data(timeString), std::size(timeString), "%FT%TZ", std::gmtime(&now));


    QCommandLineParser CLI;
    CLI.setApplicationDescription("fylgje-saver - your mythical instrument following data storer");
    std::map<std::string, QCommandLineOption> cliOptions{
        {"file",        QCommandLineOption("f", "JSON configuration <file>.", "file")},
        {"broker",      QCommandLineOption("b", "Kafka <broker> url.", "broker"),},
        {"topic",       QCommandLineOption("t", "Kafka <topic>.", "kafka"),},
        {"config",      QCommandLineOption("k", "Kafka <configuration> file.", "configuration"),},
        {"calibration", QCommandLineOption({"c", "calibration"}, "Detector calibration JSON file", "calibration.json")},
        {"output",      QCommandLineOption({"o", "output"}, "Output file", "output.h5")},
        {"from",        QCommandLineOption("from", "Start time for accumulation", "from", timeString)},
        {"to",          QCommandLineOption("to", "End time for accumulation", "to", timeString)},
        {"duration",   QCommandLineOption("duration", "Duration for accumulation", "duration", "1h")},
        {"verbose",     QCommandLineOption("v", "Verbose output")},
    };
    CLI.addHelpOption();
    for (const auto & [name, opt]: cliOptions) {
      CLI.addOption(opt);
    }
    CLI.process(app);

    Configuration Config;
    if (CLI.isSet(cliOptions.at("file"))) {
      if (auto fileName = CLI.value(cliOptions.at("file")).toStdString(); !fileName.empty()) {
        Config.fromJsonFile(fileName);
      }
    }
    if (CLI.isSet(cliOptions.at("broker"))) {
      if (auto broker = CLI.value(cliOptions.at("broker")).toStdString(); !broker.empty()) {
        Config.Kafka.Broker = broker;
        std::cout << fmt::format("<<<<\n WARNING Override kafka broker to {} \n>>>>\n", Config.Kafka.Broker);
      }
    }
    if (CLI.isSet(cliOptions.at("topic"))) {
      if (auto topic = CLI.value(cliOptions.at("topic")).toStdString(); !topic.empty()) {
        Config.Kafka.Topic = topic;
        std::cout << fmt::format("<<<<\n WARNING Override kafka topic to {} \n>>>>\n", Config.Kafka.Topic);
      }
    }
    if (CLI.isSet(cliOptions.at("config"))) {
      if (auto config = CLI.value(cliOptions.at("config")).toStdString(); !config.empty()){
        Config.KafkaConfigFile = config;
      }
    }

    Calibration calibration{};
    if (CLI.isSet(cliOptions.at("calibration"))) {
      if (auto calib = CLI.value(cliOptions.at("calibration")).toStdString(); !calib.empty()) {
        calibration = from_json_file(calib);
      }
    }

    std::string output_file{"output.h5"};
    if (CLI.isSet(cliOptions.at("output"))) {
      if (auto output = CLI.value(cliOptions.at("output")).toStdString(); !output.empty()) {
        output_file = output;
      }
    }

    auto from_set{CLI.isSet(cliOptions.at("from"))};
    auto to_set{CLI.isSet(cliOptions.at("to"))};
    auto duration_set{CLI.isSet(cliOptions.at("duration"))};
    std::time_t from_time{now}, to_time{now};
    if (from_set && to_set && duration_set) {
      std::cout << "Setting all of from, to, and duration is likely to cause inconsistencies. Duration ignored\n";
      duration_set = false;
    }
    if (from_set) {
      from_time = string_to_time_t(CLI.value(cliOptions.at("from")).toStdString());
    }
    if (to_set) {
      to_time = string_to_time_t(CLI.value(cliOptions.at("to")).toStdString());
    }
    if (duration_set){
      auto duration = duration_string_to_seconds(CLI.value(cliOptions.at("duration")).toStdString());
      if (from_set) {
        to_time = from_time + duration.count();
      } else {
        from_time = to_time - duration.count();
      }
    }

    do_work(Config, calibration, from_time, to_time, output_file);
}
