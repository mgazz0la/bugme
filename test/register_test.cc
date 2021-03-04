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

    a.increment();;
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

TEST(WordRegisterTest, Overflows) {
  gbc::WordRegister a;
  a.set(UINT16_MAX);
  EXPECT_EQ(UINT16_MAX, a.value());
  a.increment();
  EXPECT_EQ(0, a.value());
}

TEST(WordRegisterTest, Underflows) {
  gbc::WordRegister a;
  a.set(0);
  EXPECT_EQ(0, a.value());
  a.decrement();
  EXPECT_EQ(UINT16_MAX, a.value());
}

TEST(WordRegisterTest, HighLow) {
  gbc::WordRegister a;
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
    EXPECT_EQ(i & 0xFF, ab.low());;
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
