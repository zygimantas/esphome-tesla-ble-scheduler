// The board's own checks on a settings file, for fake_board.py, built like the unit tests (CONTRIBUTING.md's Checks)
// and run from the repository root, as it reads plans/:
//   settings_tool options                      what GET /settings/options answers
//   settings_tool check <file> [<file before>] what's wrong with the file, as POST /settings answers it, exiting 1, or
//                                              nothing; with the file the board had, "restarts" and "deletes schedule"
//                                              on lines of their own where saving over it would (settings.h)
#include "repository_plans.h"
#include "scheduler/settings.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

using namespace esphome::scheduler;
using scheduler_test::repository_plans;

static std::string read_file(const char *path) {
  std::ifstream file(path);
  std::stringstream text;
  text << file.rdbuf();
  return text.str();
}

int main(int argc, char **argv) {
  const std::string command = argc > 1 ? argv[1] : "";
  if (command == "options" && argc == 2) {
    settings_options(repository_plans(), [](const std::string &piece) { std::fputs(piece.c_str(), stdout); });
    return 0;
  }
  if (command == "check" && (argc == 3 || argc == 4)) {
    SettingsFile file, was;
    const std::string error = read_settings(read_file(argv[2]), repository_plans(), file);
    if (!error.empty()) {
      std::puts(error.c_str());
      return 1;
    }
    if (argc == 4) {
      read_settings(read_file(argv[3]), repository_plans(), was);  // none if it doesn't read, as on the board
      if (restarts(was, file))
        std::puts("restarts");
      if (deletes_schedule(was, file))
        std::puts("deletes schedule");
    }
    return 0;
  }
  std::fputs("Usage: settings_tool options | check <file> [<file before>]\n", stderr);
  return 2;
}
