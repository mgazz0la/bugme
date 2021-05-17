#include <csignal>
#include <iostream>

#include "error.hh"
#include "gbc.hh"
#include "options.hh"

namespace {
bugme::Gbc *global_gbc_pointer;
}

int main(int argc, char **argv) {
  bugme::CliOptions cli_options = bugme::get_cli_options(argc, argv);
  bugme::Gbc gbc(cli_options);
  ::global_gbc_pointer = &gbc;
  signal(SIGINT,
         [](int) { ::global_gbc_pointer->exit(bugme::exit::EXIT_SIGINT); });
  return gbc.run();
}
