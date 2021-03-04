#ifndef GBC_CPU_H
#define GBC_CPU_H

#include "register.h"

namespace gbc {

class CPU {
public:
  CPU();

private:
  ByteRegister a, b, c, d, e, h, l;
  ByteRegisterPair af, bc, de, hl;
  WordRegister pc;
  WordRegister sp;
};

} // namespace gbc
#endif
