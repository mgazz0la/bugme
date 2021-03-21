/*
Copyright (c) 2015-2019 Jonathan Gilchrist
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of gbemu nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GBC_CLI_H
#define GBC_CLI_H

#include "options.hh"
#include "log.hh"

#include <string>
#include <vector>

namespace gbc {

CliOptions get_cli_options(int argc, char *argv[]);
CliOptions get_cli_options(int argc, char *argv[]) {
  if (argc < 2) {
    log_error("Please provide a ROM file to run");
  }

  CliOptions cliOptions;
  cliOptions.rom_filename = argv[1];

  std::vector<std::string> flags(argv + 2, argv + argc);

/*
  for (std::string &flag : flags) {
    if (flag == "--debug") {
      cliOptions.options.debugger = true;
    } else if (flag == "--trace") {
      cliOptions.options.trace = true;
    } else if (flag == "--silent") {
      cliOptions.options.disable_logs = true;
    } else if (flag == "--headless") {
      cliOptions.options.headless = true;
    } else if (flag == "--whole-framebuffer") {
      cliOptions.options.show_full_framebuffer = true;
    } else if (flag == "--exit-on-infinite-jr") {
      cliOptions.options.exit_on_infinite_jr = true;
    } else if (flag == "--print-serial") {
      cliOptions.options.print_serial = true;
    } else {
      fatal_error("Unknown flag: %s", flag.c_str());
    }
  }
  */

  return cliOptions;
}
} // namespace gbc

#endif
