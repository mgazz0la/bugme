#ifndef BUGME_OPTIONS_HH
#define BUGME_OPTIONS_HH

#include <string>

namespace bugme {
struct Options {
  bool debug = false;
  int verbosity = 0;
  bool headless = false;
};

struct CliOptions {
  Options options;
  std::string rom_filename;
};

extern CliOptions get_cli_options(int argc, char *argv[]);
}  // namespace bugme
#endif
