#ifndef GBC_REGISTER_H
#define GBC_REGISTER_H

#include <cstdint>

class ByteRegister {
public:
  uint8_t value() const;

protected:
  uint8_t val = 0x0;
};

#endif
