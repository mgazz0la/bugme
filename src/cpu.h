#ifndef GBC_CPU_H
#define GBC_CPU_H

#include "opcode.h"
#include "register.h"

namespace gbc {

class Cpu {
public:
  Cpu();

private:
  ByteRegister a, b, c, d, e, h, l;
  ByteRegisterPair af, bc, de, hl;
  WordRegister pc;
  WordRegister sp;

  // TODO: Make a separate class for the flags register.
  ByteRegister f;
};

} // namespace gbc
#endif
