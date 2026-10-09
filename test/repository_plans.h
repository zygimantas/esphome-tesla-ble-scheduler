#pragma once
// The plans in plans/, read from the repository root, for the unit tests and settings_tool.cpp.

#include "scheduler/settings.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace scheduler_test {

// The plans in plans/, by name like lt/eso-standartinis-4-zones, as the board has them built in.
inline const esphome::scheduler::Plans &repository_plans() {
  static std::vector<std::pair<std::string, std::string>> texts;
  static esphome::scheduler::Plans plans;
  if (plans.empty()) {
    for (const auto &entry : std::filesystem::recursive_directory_iterator("plans")) {
      if (entry.path().extension() != ".yaml")
        continue;
      std::ifstream file(entry.path());
      std::stringstream text;
      text << file.rdbuf();
      texts.emplace_back(entry.path().lexically_relative("plans").replace_extension().string(), text.str());
    }
    std::sort(texts.begin(), texts.end());
    for (const auto &[name, text] : texts)
      plans.emplace_back(name, text);
  }
  return plans;
}

}  // namespace scheduler_test
