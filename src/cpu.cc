#include "cpu.h"

namespace gbc {

Cpu::Cpu() : af(a, f), bc(b, c), de(d, e), hl(h, l) {}

}
