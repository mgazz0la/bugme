#include <iostream>

#include "cli.hh"
#include "gbc.hh"

int main(int argc, char **argv) {
  gbc::CliOptions cli_options = gbc::get_cli_options(argc, argv);
  gbc::Gbc gbc(cli_options);
  return gbc.run();
}
