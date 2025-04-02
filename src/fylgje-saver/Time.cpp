#include <iostream>
#include <iomanip>
#include <chrono>
#include <fmt/format.h>

#include "Time.h"

std::time_t now_to_time_t() {
  return std::time(nullptr);
}

std::time_t string_to_time_t(const std::string & time_str) {
  struct std::tm tm{};
  std::istringstream ss(time_str);
  ss >> std::get_time(&tm, "%Y-%m-%dT%TZ");
  if (ss.fail()){
    throw std::runtime_error(fmt::format("Failed to parse UTC time from '{}'", time_str));
  }
  auto time = timegm(&tm);
  if (ss.peek() == '.') {
    ss.ignore();
    double fractional;
    ss >> fractional;
    if (ss.fail()) {
      std::cout << fmt::format("Failed to parse fractional seconds from '{}'", time_str) << std::endl;
    } else {
      auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double>(fractional));
      auto in_seconds = std::chrono::duration_cast<std::chrono::seconds>(microseconds);
      time += std::chrono::duration_cast<std::chrono::seconds>(microseconds).count();
    }
  }
  return time;
}

std::string time_t_to_string(std::time_t time) {
  char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
  std::strftime(std::data(timeString), std::size(timeString), "%FT%TZ", std::gmtime(&time));
  return timeString;
}


std::chrono::duration<long> duration_string_to_seconds(const std::string & duration_str){
  if (duration_str.empty()){
    return std::chrono::seconds(0);
  }
  if (duration_str.back() == 'd'){
    auto days = std::chrono::hours(24 * std::stoi(duration_str));
    return std::chrono::duration_cast<std::chrono::seconds>(days);
  }
  if (duration_str.back() == 'h'){
    auto hours = std::chrono::hours(std::stoi(duration_str));
    return std::chrono::duration_cast<std::chrono::seconds>(hours);
  }
  if (duration_str.back() == 'm'){
    auto minutes = std::chrono::minutes(std::stoi(duration_str));
    return std::chrono::duration_cast<std::chrono::seconds>(minutes);
  }
  if (duration_str.back() == 's'){
    return std::chrono::seconds(std::stoi(duration_str));
  }
  std::cout << "Unknown duration string " << duration_str << std::endl;
  return std::chrono::seconds(0);
}