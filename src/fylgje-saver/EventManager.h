#pragma once
#include <map>
#include <vector>
#include <fmt/format.h>
#include <h5cpp/hdf5.hpp>
#include "Message.h"
#include "PixelManager.h"
#include "HistogramManager.h"

namespace bifrost::data {
  class EventManager {
  public:
    using message_t = bifrost::message_t;
  private:
    std::vector<message_t> messages;
    PixelManager pixel_manager;
    std::optional<HistogramManager> histogram_manager;
    bool store_pixels;

  public:
    EventManager(PixelManager && pixelManager, HistogramManager && histogramManager, bool storePixels=true)
      : pixel_manager{std::move(pixelManager)}, histogram_manager{std::move(histogramManager)}, store_pixels{storePixels} {}
    EventManager(const PixelManager & pixelManager, const HistogramManager & histogramManager, bool storePixels=true)
      : pixel_manager{pixelManager}, histogram_manager{histogramManager}, store_pixels{storePixels} {}

    EventManager(PixelManager && pixelManager, bool storePixels=true)
        : pixel_manager{std::move(pixelManager)}, store_pixels{storePixels} {}
    EventManager(const PixelManager & pixelManager, bool storePixels=true)
        : pixel_manager{pixelManager}, store_pixels{storePixels} {}

    // / \brief Add a message to the event manager from a message_t
    bool add(const message_t & message);
    // / \brief Add a message to the event manager from its components
    bool add(int fiber, int group, int a, int b, double time, uint32_t high, uint32_t low);

    bool storePixels(bool store=true) {
      store_pixels = store;
      return store_pixels;
    }
    void clear() {
      if (store_pixels){
        pixel_manager.clear();
      }
      if (histogram_manager.has_value()){
        histogram_manager->clear();
      }
    }

    const HistogramManager & histograms() const {
      if (histogram_manager.has_value()){
        return *histogram_manager;
      }
      throw std::runtime_error("No histogram manager available");
    }
    HistogramManager & histograms() {
      if (histogram_manager.has_value()){
        return *histogram_manager;
      }
      throw std::runtime_error("No histogram manager available");
    }

    void save_to(hdf5::node::Group group) const;
    void save_to(hdf5::file::File file, std::optional<std::string> group = std::nullopt) const;
    void save_to(std::filesystem::path file, std::optional<std::string> group = std::nullopt) const;
  };
}