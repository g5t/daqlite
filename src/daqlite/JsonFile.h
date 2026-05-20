// Copyright (C) 2019 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file JsonFile.h
///
/// \brief Reads JSON files into nlohmann types
///
/// See https://nlohmann.github.io/json/doxygen/index.html
//===----------------------------------------------------------------------===//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <nlohmann/json.hpp>
#pragma GCC diagnostic pop

#include <fmt/format.h>

#include <exception>
#include <fstream>
#include <stdexcept>
#include <string>

/// \brief Read a JSON file from disk and parse it into an nlohmann::json
/// \param fname  Path to the JSON file
/// \return Parsed JSON object
/// \throws std::runtime_error if the file cannot be opened or is not valid JSON
inline nlohmann::json readJsonFile(const std::string &fname) {
  // Open the file and bail out clearly on missing path or permission errors
  std::ifstream ifs(fname);
  if (ifs.fail()) {
    throw std::runtime_error(
        fmt::format("could not open JSON file {}", fname));
  }

  // Parse the stream; rethrow nlohmann's parse_error with file context
  nlohmann::json Parsed;
  try {
    ifs >> Parsed;
  } catch (const std::exception &e) {
    throw std::runtime_error(
        fmt::format("file {} is not valid JSON: {}", fname, e.what()));
  }

  return Parsed;
}
