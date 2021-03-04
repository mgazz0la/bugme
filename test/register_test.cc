#include "register.h"
#include <cstdint>
#include <gtest/gtest.h>

TEST(ByteRegisterTest, SetsValue) {
  uint8_t val = 69;
  gbc::ByteRegister a;
  a.set(val);

  EXPECT_EQ(val, a.value());
}
