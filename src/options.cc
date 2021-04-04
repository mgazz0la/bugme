#include "options.hh"

#include "log.hh"

#include <string>
#include <vector>

namespace gbc {
CliOptions get_cli_options(int argc, char *argv[]) {
  if (argc < 2) {
    log_error("ROM file required");
  }

  CliOptions cliOptions;
  cliOptions.rom_filename = argv[1];

  std::vector<std::string> flags(argv + 2, argv + argc);

  for (unsigned int i = 0; i < flags.size(); ++i) {
    if (flags[i] == "--debug") {
      cliOptions.options.debug = true;
    } else if (flags[i] == "-v") {
      int verbosity = std::atoi(flags[i + 1].c_str());
      // bound it to 0-3
      cliOptions.options.verbosity =
          verbosity >= 0 ? (verbosity <= 3 ? verbosity : 3) : 0;
      ++i;
    } else if (flags[i] == "--headless") {
      cliOptions.options.headless = true;
    } else {
      log_error("Unknown flag: %s", flags[i].c_str());
    }
  }
  return cliOptions;
}
} // namespace gbc
