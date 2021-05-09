add_library(cpu cpu.cc opcode.cc opcode_internal.cc)
target_link_libraries(cpu LINK_PRIVATE joypad log memory ppu timer)
