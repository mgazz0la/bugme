#include <iostream>

#include "gbc.hh"
#include "cli.hh"

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  gbc::CliOptions cli_options = gbc::get_cli_options(argc, argv);
  gbc::Gbc gbc(cli_options);
  gbc.start();
  return 0;
}
