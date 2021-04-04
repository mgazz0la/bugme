#include <csignal>
#include <iostream>

#include "gbc.hh"
#include "options.hh"

namespace {
gbc::Gbc *global_gbc_pointer;
}

int main(int argc, char **argv) {
  gbc::CliOptions cli_options = gbc::get_cli_options(argc, argv);
  gbc::Gbc gbc(cli_options);
  ::global_gbc_pointer = &gbc;
  signal(SIGINT, [](int) { ::global_gbc_pointer->exit(); });
  return gbc.run();
}
