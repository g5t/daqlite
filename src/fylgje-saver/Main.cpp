#include <iostream>
#include <map>
#include <fmt/format.h>

#include "args.hxx"

#include "Configuration.h"
#include "Calibration.h"
#include "Time.h"
#include "ESSConsumer.h"
#include "EventManager.h"
#include "HistogramManager.h"
#include "PixelManager.h"

using namespace kafka::time;

void do_work(Configuration & configuration, Calibration & calibration, milliseconds from, milliseconds to, const std::string& output_file) {
  using namespace bifrost::data;
  auto tubes = configuration.Instrument.units_per_group;
  auto pixelation = configuration.Instrument.pixels_per_unit;
  int arcs{5}, triplets{9};
  bool store_pixels{true};
  auto data = std::make_shared<EventManager>(PixelManager(arcs, triplets, tubes, pixelation, calibration), HistogramManager(arcs ,triplets, calibration), store_pixels);

  // auto just_events = std::make_shared<EventManager>(PixelManager(arcs, triplets, tubes, pixelation, calibration), false);

  ESSConsumer worker{data, configuration, from, to};
  worker.run();
  data->save_to(output_file);
}

int main(int argc, char *argv[]){
    args::ArgumentParser parser("Save data from Kafka to HDF5",
                                "Uses fylgje internals to grab messages.");

    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::Flag verbose(parser, "verbose", "Print additional information", {'v', "verbose"});

    auto now = std::time({});
    char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
    std::strftime(std::data(timeString), std::size(timeString), "%FT%TZ", std::gmtime(&now));
    Configuration Config;
    Calibration calibration;

    args::ValueFlag<std::string> file_flag(parser, "file", "JSON configuration <file>.", {'f', "file"});
    args::ValueFlag<std::string> broker_flag(parser, "broker", "Kafka <broker> url.", {'b', "broker"});
    args::ValueFlag<std::string> topic_flag(parser, "topic", "Kafka <topic>.", {'t', "topic"});
    args::ValueFlag<std::string> config_flag(parser, "config", "Kafka <configuration> file.", {'k', "config"});
    args::ValueFlag<std::string> calibration_flag(parser, "calibration", "Detector calibration JSON file", {'c', "calibration"});
    args::ValueFlag<std::string> output_flag(parser, "output", "Output file", {'o', "output"});
    args::ValueFlag<std::string> from_flag(parser, "from", "Start time for accumulation", {"from"}, timeString);
    args::ValueFlag<std::string> to_flag(parser, "to", "End time for accumulation", {"to"}, timeString);
    args::ValueFlag<std::string> duration_flag(parser, "duration", "Duration for accumulation", {"duration"}, "1h");

    try {
      parser.ParseCLI(argc, argv);
    }
    catch (const args::Help&) {
      std::cout << parser;
      return 0;
    }
    catch (const args::ParseError& e) {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      return 1;
    }

    if (file_flag) {
      if (auto file = args::get(file_flag); !file.empty()) {
        Config.fromJsonFile(file);
      }
    }
    if (broker_flag){
      if (auto broker = args::get(broker_flag); !broker.empty()) {
        Config.Kafka.Broker = broker;
        std::cout << fmt::format("<<<<\n WARNING Override kafka broker to {} \n>>>>\n", Config.Kafka.Broker);
      }
    }
    if (topic_flag){
      if (auto topic = args::get(topic_flag); !topic.empty()) {
        Config.Kafka.Topic = topic;
        std::cout << fmt::format("<<<<\n WARNING Override kafka topic to {} \n>>>>\n", Config.Kafka.Topic);
      }
    }
    if (config_flag){
      if (auto config = args::get(config_flag); !config.empty()) {
        Config.KafkaConfigFile = config;
      }
    }
    if (calibration_flag){
      if (auto calib = args::get(calibration_flag); !calib.empty()) {
        calibration = from_json_file(calib);
      }
    }
    std::string output_file{"output.h5"};
    if (output_flag){
      if (auto output = args::get(output_flag); !output.empty()) {
        output_file = output;
      }
    }

    milliseconds from_time{time_t_to_milliseconds(now)};
    auto to_time=from_time;

    if (from_flag && to_flag && duration_flag) {
      std::cout << "Setting all of from, to, and duration is likely to cause inconsistencies. Duration ignored\n";
    }
    if (from_flag) {
      from_time = time_string_to_milliseconds(args::get(from_flag));
    }
    if (to_flag) {
      to_time = time_string_to_milliseconds(args::get(to_flag));
    }
    if (duration_flag && (from_flag ^ to_flag)){
      auto duration = duration_string_to_milliseconds(args::get(duration_flag));
      if (from_flag) {
        to_time = from_time + duration;
      } else {
        from_time = to_time - duration;
      }
    }

    do_work(Config, calibration, from_time, to_time, output_file);
}


