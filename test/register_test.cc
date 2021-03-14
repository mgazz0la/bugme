#include "register.hh"
#include <cstdint>
#include <gtest/gtest.h>

TEST(ByteRegisterTest, Overflows) {
  gbc::ByteRegister a;
  a.set(UINT8_MAX);
  EXPECT_EQ(UINT8_MAX, a.value());

  a.increment();
  EXPECT_EQ(0, a.value());
}

TEST(ByteRegisterTest, Underflows) {
  gbc::ByteRegister a;
  a.set(0);
  EXPECT_EQ(0, a.value());

  a.decrement();
  EXPECT_EQ(UINT8_MAX, a.value());
}

TEST(ByteRegisterTest, GetBit) {
  gbc::ByteRegister a;

  for (std::uint8_t i = 0; i < UINT8_MAX; ++i) {
    EXPECT_EQ(i, a.value());

    for (int j = 0; j < 8; ++j) {
      EXPECT_EQ((i >> j) & 1, a.get_bit(j));
    }

    a.increment();
    ;
  }

  EXPECT_EQ(UINT8_MAX, a.value());
  for (int j = 0; j < 8; ++j) {
    EXPECT_EQ(1, a.get_bit(j));
  }
}

TEST(ByteRegisterTest, WriteBit) {
  gbc::ByteRegister a;
  a.reset();

  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 8; ++j) {
      a.write_bit(j, !!(i & (1 << j)));
    }
    EXPECT_EQ(i, a.value());
  }
}

TEST(ByteRegisterTest, SetBitClearBit) {
  gbc::ByteRegister a;
  a.reset();

  for (int i = 0; i < 8; ++i) {
    a.set_bit(i);
    EXPECT_EQ(1 << i, a.value());
    a.clear_bit(i);
  }
}

TEST(ByteRegisterTest, FlipBit) {
  gbc::ByteRegister a;
  a.reset();

  for (int i = 0; i < 8; ++i) {
    a.flip_bit(i);
    EXPECT_EQ(1 << i, a.value());
    a.flip_bit(i);
  }
}

TEST(ByteRegisterTest, Equals) {
  gbc::ByteRegister a, b;
  a.reset();
  b.reset();

  for (std::uint8_t i = 0; i < UINT8_MAX; ++i) {
    EXPECT_EQ(a, b);
    a.increment();
    b.increment();
  }

  EXPECT_EQ(a, b);
}

TEST(ByteRegisterTest, DoesNotEqual) {
  gbc::ByteRegister a, b;
  a.reset();
  b.set(1);
  EXPECT_FALSE(a == b);
}

TEST(WordValuedRegisterTest, Overflows) {
  gbc::WordValuedRegister a;
  a.set(UINT16_MAX);
  EXPECT_EQ(UINT16_MAX, a.value());
  a.increment();
  EXPECT_EQ(0, a.value());
}

TEST(WordValuedRegisterTest, Underflows) {
  gbc::WordValuedRegister a;
  a.set(0);
  EXPECT_EQ(0, a.value());
  a.decrement();
  EXPECT_EQ(UINT16_MAX, a.value());
}

TEST(WordValuedRegisterTest, HighLow) {
  gbc::WordValuedRegister a;
  a.set(0);

  for (std::uint16_t i = 0; i < UINT16_MAX; ++i) {
    EXPECT_EQ(i & 0xFF, a.low());
    EXPECT_EQ((i >> 8) & 0xFF, a.high());
    a.increment();
  }

  EXPECT_EQ(0xFF, a.low());
  EXPECT_EQ(0xFF, a.high());
}

TEST(ByteRegisterPairTest, Sets) {
  gbc::ByteRegister a, b;
  a.reset();
  b.reset();

  gbc::ByteRegisterPair ab(a, b);

  for (std::uint16_t i = 0; i < UINT16_MAX; ++i) {
    EXPECT_EQ(i, ab.value());
    EXPECT_EQ((i >> 8) & 0xFF, ab.high());
    EXPECT_EQ(i & 0xFF, ab.low());
    ;
    ab.increment();
  }

  EXPECT_EQ(UINT16_MAX, ab.value());
  EXPECT_EQ(0xFF, ab.high());
  EXPECT_EQ(0xFF, ab.low());
}

TEST(ByteRegisterPairTest, Overflows) {
  gbc::ByteRegister a, b;
  a.reset();
  b.reset();

  gbc::ByteRegisterPair ab(a, b);
  ab.set(UINT16_MAX);
  EXPECT_EQ(UINT16_MAX, ab.value());
  ab.increment();
  EXPECT_EQ(0, ab.value());
}

TEST(ByteRegisterPairTest, Underflows) {
  gbc::ByteRegister a, b;
  a.reset();
  b.reset();

  gbc::ByteRegisterPair ab(a, b);
  ab.set(0);
  EXPECT_EQ(0, ab.value());
  ab.decrement();
  EXPECT_EQ(UINT16_MAX, ab.value());
}

TEST(FlagRegisterTest, SetsFlags) {
  gbc::FlagRegister f;
  f.reset();

  EXPECT_EQ(0, f.zero_flag());
  EXPECT_EQ(0, f.subtract_flag());
  EXPECT_EQ(0, f.half_carry_flag());
  EXPECT_EQ(0, f.carry_flag());

  f.set_zero_flag();
  f.set_subtract_flag();
  f.set_half_carry_flag();
  f.set_carry_flag();

  EXPECT_EQ(1, f.zero_flag());
  EXPECT_EQ(1, f.subtract_flag());
  EXPECT_EQ(1, f.half_carry_flag());
  EXPECT_EQ(1, f.carry_flag());
}

TEST(FlagRegisterTest, ClearsFlags) {
  gbc::FlagRegister f;
  f.reset();

  f.set_zero_flag();
  f.set_subtract_flag();
  f.set_half_carry_flag();
  f.set_carry_flag();

  EXPECT_EQ(1, f.zero_flag());
  EXPECT_EQ(1, f.subtract_flag());
  EXPECT_EQ(1, f.half_carry_flag());
  EXPECT_EQ(1, f.carry_flag());

  f.clear_zero_flag();
  f.clear_subtract_flag();
  f.clear_half_carry_flag();
  f.clear_carry_flag();

  EXPECT_EQ(0, f.zero_flag());
  EXPECT_EQ(0, f.subtract_flag());
  EXPECT_EQ(0, f.half_carry_flag());
  EXPECT_EQ(0, f.carry_flag());
}

TEST(FlagRegisterTest, FlipsFlags) {
  gbc::FlagRegister f;
  f.reset();

  EXPECT_EQ(0, f.zero_flag());
  EXPECT_EQ(0, f.subtract_flag());
  EXPECT_EQ(0, f.half_carry_flag());
  EXPECT_EQ(0, f.carry_flag());

  f.flip_zero_flag();
  f.flip_subtract_flag();
  f.flip_half_carry_flag();
  f.flip_carry_flag();

  EXPECT_EQ(1, f.zero_flag());
  EXPECT_EQ(1, f.subtract_flag());
  EXPECT_EQ(1, f.half_carry_flag());
  EXPECT_EQ(1, f.carry_flag());

  f.flip_zero_flag();
  f.flip_subtract_flag();
  f.flip_half_carry_flag();
  f.flip_carry_flag();

  EXPECT_EQ(0, f.zero_flag());
  EXPECT_EQ(0, f.subtract_flag());
  EXPECT_EQ(0, f.half_carry_flag());
  EXPECT_EQ(0, f.carry_flag());
}

TEST(FlagRegisterTest, WritesFlags) {
  gbc::FlagRegister f;
  f.reset();

  EXPECT_EQ(0, f.zero_flag());
  EXPECT_EQ(0, f.subtract_flag());
  EXPECT_EQ(0, f.half_carry_flag());
  EXPECT_EQ(0, f.carry_flag());

  f.write_zero_flag(1);
  f.write_subtract_flag(1);
  f.write_half_carry_flag(1);
  f.write_carry_flag(1);

  EXPECT_EQ(1, f.zero_flag());
  EXPECT_EQ(1, f.subtract_flag());
  EXPECT_EQ(1, f.half_carry_flag());
  EXPECT_EQ(1, f.carry_flag());

  f.write_zero_flag(0);
  f.write_subtract_flag(0);
  f.write_half_carry_flag(0);
  f.write_carry_flag(0);

  EXPECT_EQ(0, f.zero_flag());
  EXPECT_EQ(0, f.subtract_flag());
  EXPECT_EQ(0, f.half_carry_flag());
  EXPECT_EQ(0, f.carry_flag());
}

TEST(FlagRegisterTest, DoesNotWriteToLowerNibble) {
  gbc::FlagRegister f;

  f.reset();
  EXPECT_EQ(0, f.get_bit(0));
  EXPECT_EQ(0, f.get_bit(1));
  EXPECT_EQ(0, f.get_bit(2));
  EXPECT_EQ(0, f.get_bit(3));

  f.increment();
  EXPECT_EQ(0, f.get_bit(0));
  EXPECT_EQ(0, f.get_bit(1));
  EXPECT_EQ(0, f.get_bit(2));
  EXPECT_EQ(0, f.get_bit(3));

  f.decrement();
  EXPECT_EQ(0, f.get_bit(0));
  EXPECT_EQ(0, f.get_bit(1));
  EXPECT_EQ(0, f.get_bit(2));
  EXPECT_EQ(0, f.get_bit(3));

  f.set(0xFF);
  EXPECT_EQ(0, f.get_bit(0));
  EXPECT_EQ(0, f.get_bit(1));
  EXPECT_EQ(0, f.get_bit(2));
  EXPECT_EQ(0, f.get_bit(3));

  f.set_bit(0);
  f.set_bit(1);
  f.set_bit(2);
  f.set_bit(3);
  EXPECT_EQ(0, f.get_bit(0));
  EXPECT_EQ(0, f.get_bit(1));
  EXPECT_EQ(0, f.get_bit(2));
  EXPECT_EQ(0, f.get_bit(3));

  f.flip_bit(0);
  f.flip_bit(1);
  f.flip_bit(2);
  f.flip_bit(3);
  EXPECT_EQ(0, f.get_bit(0));
  EXPECT_EQ(0, f.get_bit(1));
  EXPECT_EQ(0, f.get_bit(2));
  EXPECT_EQ(0, f.get_bit(3));

  f.write_bit(0, 1);
  f.write_bit(1, 1);
  f.write_bit(2, 1);
  f.write_bit(3, 1);
  EXPECT_EQ(0, f.get_bit(0));
  EXPECT_EQ(0, f.get_bit(1));
  EXPECT_EQ(0, f.get_bit(2));
  EXPECT_EQ(0, f.get_bit(3));
}
