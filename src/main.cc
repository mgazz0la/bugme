#include <iostream>

#include "gbc.hh"

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  gbc::Gbc gbc;
  std::cout << gbc.status();
  return 0;
}
