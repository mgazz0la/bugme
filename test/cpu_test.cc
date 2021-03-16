#include "cpu.hh"
#include "mmu.hh"
#include "types.hh"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <optional>

#define FLAG_Z (0b10000000)
#define FLAG_S (0b01000000)
#define FLAG_H (0b00100000)
#define FLAG_C (0b00010000)

namespace gbc {

using ::testing::_;
using ::testing::Return;

class MockMmu : public Mmu {
public:
  MOCK_METHOD(byte_t, read, (word_t addr), (const override));
  MOCK_METHOD(void, write, (word_t addr, byte_t byte), (override));
};

class CpuTest : public ::testing::Test {
protected:
  struct CpuState {
    std::optional<byte_t> a = std::nullopt;
    std::optional<byte_t> f = std::nullopt;
    std::optional<word_t> af = std::nullopt;

    std::optional<byte_t> b = std::nullopt;
    std::optional<byte_t> c = std::nullopt;
    std::optional<word_t> bc = std::nullopt;

    std::optional<byte_t> d = std::nullopt;
    std::optional<byte_t> e = std::nullopt;
    std::optional<word_t> de = std::nullopt;

    std::optional<byte_t> h = std::nullopt;
    std::optional<byte_t> l = std::nullopt;
    std::optional<word_t> hl = std::nullopt;

    std::optional<word_t> sp = std::nullopt;
    std::optional<word_t> pc = std::nullopt;

    std::optional<bool> stopped_ = std::nullopt;
    std::optional<bool> halted_ = std::nullopt;
    std::optional<bool> did_branch_ = std::nullopt;

    bool operator==(std::shared_ptr<Cpu> o) const {
      if (((a || f) && af) || ((b || c) && bc) || ((d || e) && de) ||
          ((h || l) && hl)) {
        throw "Bad CpuState";
      }

      return (af ? af == o->af
                 : ((a ? a == o->a : o->a == 0) &&
                    (f ? f == o->f : o->f == 0))) &&
             (bc ? bc == o->bc
                 : ((b ? b == o->b : o->b == 0) &&
                    (c ? c == o->c : o->c == 0))) &&
             (de ? de == o->de
                 : ((d ? d == o->d : o->d == 0) &&
                    (e ? e == o->e : o->e == 0))) &&
             (hl ? hl == o->hl
                 : ((h ? h == o->h : o->h == 0) &&
                    (l ? l == o->l : o->l == 0))) &&
             (sp ? sp == o->sp : o->sp == 0) &&
             (pc ? pc == o->pc : o->pc == 0) &&
             (stopped_ ? stopped_ == o->stopped_ : o->stopped_ == false) &&
             (halted_ ? halted_ == o->halted_ : o->halted_ == false) &&
             (did_branch_ ? did_branch_ == o->did_branch_
                          : o->did_branch_ == false);
    }
  };

  const word_t WORD = 0x420;
  const word_t ADDR = 0x420;
  const byte_t BYTE = 0x69;

  std::shared_ptr<MockMmu> mmu;
  std::shared_ptr<Cpu> cpu;

  CpuTest() : mmu(new MockMmu()), cpu(new Cpu(mmu)) {}

  void SetUp() override { cpu->reset(); }
};

TEST_F(CpuTest, op_00) { // NOP
  cpu->op_00();

  CpuState expected_state = {/* no changes */};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_01) { // LD BC,d16
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1))
      .WillOnce(Return(0xFF & WORD));
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 2))
      .WillOnce(Return((WORD >> 8) & 0xFF));

  cpu->op_01();

  CpuState expected_state = {.bc = WORD};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_02) { // LD (BC),A
  cpu->a.set(BYTE);
  cpu->bc.set(ADDR);
  EXPECT_CALL(*mmu, write(ADDR, BYTE)).Times(1);

  cpu->op_02();

  CpuState expected_state = {.a = BYTE, .bc = ADDR};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_03) { // INC BC
  cpu->op_03();

  CpuState expected_state = {.bc = 1};
  EXPECT_EQ(expected_state, cpu);

  // overflow
  cpu->bc.set(0xFFFF);
  cpu->op_03();

  expected_state = {.bc = 0};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_04) { // INC B
  cpu->f.set(FLAG_S);    // unsets
  cpu->op_04();

  CpuState expected_state = {.b = 1};
  EXPECT_EQ(expected_state, cpu);

  // half carry flag
  cpu->b.set(0x0F);
  cpu->op_04();

  expected_state = {.f = FLAG_H, .b = 0x10};
  EXPECT_EQ(expected_state, cpu);

  // zero flag
  cpu->b.set(0xFF);
  cpu->op_04();

  expected_state = {.f = FLAG_Z | FLAG_H, .b = 0};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_05) { // DEC B
  cpu->op_05();
  // subtract flag always set

  // half carry flag
  CpuState expected_state = {.f = FLAG_S | FLAG_H, .b = 0xFF};
  EXPECT_EQ(expected_state, cpu);

  // zero flag
  cpu->b.set(0x01);
  cpu->op_05();

  expected_state = {.f = FLAG_Z | FLAG_S, .b = 0};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_06) { // LD B,d8
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1)).WillOnce(Return(BYTE));

  cpu->op_06();

  CpuState expected_state = {.b = BYTE};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_07) {                // RLCA
  cpu->f.set(FLAG_Z | FLAG_S | FLAG_H); // unsets
  cpu->a.set(0b01000000);
  cpu->op_07();

  CpuState expected_state = {.a = 0b10000000};
  EXPECT_EQ(expected_state, cpu);

  // set carry flag when shifting msb
  cpu->op_07();

  expected_state = {.a = 0b00000001, .f = FLAG_C};
  EXPECT_EQ(expected_state, cpu);

  // doesn't set zero flag (op 0xcb 0x07 RLC A *does*, however)
  cpu->a.set(0);
  cpu->op_07();
  expected_state = {/* all zeroes */};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_08) { // LD (a16),SP
  cpu->sp.set(0xBEEF);
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1))

      .WillOnce(Return(0xFF & WORD));
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 2))

      .WillOnce(Return((WORD >> 8) & 0xFF));
  EXPECT_CALL(*mmu, write(WORD, 0xEF /* lower byte of 0xBEEF */)).Times(1);
  EXPECT_CALL(*mmu, write(WORD + 1, 0xBE /* upper byte of 0xBEEF */)).Times(1);

  cpu->op_08();
  CpuState expected_state = {.sp = 0xBEEF};
  EXPECT_EQ(expected_state, cpu);
}

// note: half carry flag on 16-bit arithmetic operators always refers
// to the higher byte
TEST_F(CpuTest, op_09) { // ADD HL,BC
  cpu->f.set(FLAG_S);    // unsets

  // sets half carry flag
  cpu->hl.set(0x0800);
  cpu->bc.set(0x0801);
  cpu->op_09();
  CpuState expected_state = {.f = FLAG_H, .bc = 0x0801, .hl = 0x1001};
  EXPECT_EQ(expected_state, cpu);

  // sets carry flag
  cpu->bc.set(0xFFFF);
  cpu->op_09();
  expected_state = {.f = FLAG_C | FLAG_H, .bc = 0xFFFF, .hl = 0x1000};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_0a) { // LD A,(BC)
  cpu->bc.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(BYTE));

  cpu->op_0a();
  CpuState expected_state = {.a = BYTE, .bc = ADDR};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_0b) { // DEC BC
  cpu->op_0b();

  // overflow
  CpuState expected_state = {.bc = 0xFFFF};
  EXPECT_EQ(expected_state, cpu);

  // normal case
  cpu->op_0b();

  expected_state = {.bc = 0xFFFE};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_0c) { // INC C
  // unsets
  cpu->f.set(FLAG_S);
  cpu->op_0c();

  CpuState expected_state = {.c = 1};
  EXPECT_EQ(expected_state, cpu);

  // half carry flag
  cpu->c.set(0x0F);
  cpu->op_0c();

  expected_state = {.f = FLAG_H, .c = 0x10};
  EXPECT_EQ(expected_state, cpu);

  // zero flag
  cpu->c.set(0xFF);
  cpu->op_0c();

  expected_state = {.f = FLAG_Z | FLAG_H, .c = 0};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_0d) { // DEC C
  cpu->op_0d();
  // subtract flag always set

  // half carry flag
  CpuState expected_state = {.f = FLAG_S | FLAG_H, .c = 0xFF};
  EXPECT_EQ(expected_state, cpu);

  // zero flag
  cpu->c.set(0x01);
  cpu->op_0d();

  expected_state = {.f = FLAG_Z | FLAG_S, .c = 0};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_0e) { // LD C,d8
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1)).WillOnce(Return(BYTE));

  cpu->op_0e();
  CpuState expected_state = {.c = BYTE};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_0f) { // RRC A
  // unsets
  cpu->f.set(FLAG_Z | FLAG_S | FLAG_H);
  cpu->a.set(0b00000010);
  cpu->op_0f();

  CpuState expected_state = {.a = 0b00000001};
  EXPECT_EQ(expected_state, cpu);

  // set carry flag when shifting msb
  cpu->op_0f();

  expected_state = {.a = 0b10000000, .f = FLAG_C};
  EXPECT_EQ(expected_state, cpu);

  // doesn't set zero flag (op 0xcb 0x0f RRC A *does*, however)
  cpu->a.set(0);
  cpu->op_0f();
  expected_state = {/* all zeroes */};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_10) { // STOP
  cpu->op_10();
  CpuState expected_state = {.stopped_ = true};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_11) { // LD DE,d16
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1))

      .WillOnce(Return(0xFF & WORD));
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 2))

      .WillOnce(Return((WORD >> 8) & 0xFF));

  cpu->op_11();

  CpuState expected_state = {.de = WORD};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_12) { // LD (DE),A
  cpu->a.set(BYTE);
  cpu->de.set(ADDR);
  EXPECT_CALL(*mmu, write(ADDR, BYTE)).Times(1);

  cpu->op_12();

  CpuState expected_state = {.a = BYTE, .de = ADDR};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_13) { // INC DE
  cpu->op_13();

  CpuState expected_state = {.de = 1};
  EXPECT_EQ(expected_state, cpu);

  // overflow
  cpu->de.set(0xFFFF);
  cpu->op_13();

  expected_state = {.de = 0};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_14) { // INC D
  // unsets
  cpu->f.set(FLAG_S);
  cpu->op_14();

  CpuState expected_state = {.d = 1};
  EXPECT_EQ(expected_state, cpu);

  // half carry flag
  cpu->d.set(0x0F);
  cpu->op_14();

  expected_state = {.f = FLAG_H, .d = 0x10};
  EXPECT_EQ(expected_state, cpu);

  // zero flag
  cpu->d.set(0xFF);
  cpu->op_14();

  expected_state = {.f = FLAG_Z | FLAG_H, .d = 0};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_15) { // DEC D
  cpu->op_15();
  // subtract flag always set

  // half carry flag
  CpuState expected_state = {.f = FLAG_S | FLAG_H, .d = 0xFF};
  EXPECT_EQ(expected_state, cpu);

  // zero flag
  cpu->d.set(0x01);
  cpu->op_15();

  expected_state = {.f = FLAG_Z | FLAG_S, .d = 0};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_16) { // LD D,d8
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1)).WillOnce(Return(BYTE));

  cpu->op_16();

  CpuState expected_state = {.d = BYTE};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_17) { // RLA
  // unsets
  cpu->f.set(FLAG_Z | FLAG_S | FLAG_H);
  cpu->a.set(0b01000000);
  cpu->op_17();

  CpuState expected_state = {.a = 0b10000000};
  EXPECT_EQ(expected_state, cpu);

  // set carry flag when shifting msb
  cpu->op_17();

  // zero flag isn't set, carry flag gets set
  expected_state = {.a = 0b00000000, .f = FLAG_C};
  EXPECT_EQ(expected_state, cpu);

  // carry flag carries over
  cpu->op_17();
  expected_state = {.a = 0b00000001};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_18) { // JR r8
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1)).WillOnce(Return(BYTE));
  cpu->op_18();
  CpuState expected_state = {.pc = static_cast<word_t>(BYTE)};
  EXPECT_EQ(expected_state, cpu);

  // treats the byte as a signed value
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1)).WillOnce(Return(~BYTE + 1));
  cpu->op_18();
  expected_state = {/* all zeroes again */};
  EXPECT_EQ(expected_state, cpu);
}

// note: half carry flag on 16-bit arithmetic operators always refers
// to the higher byte
TEST_F(CpuTest, op_19) { // ADD HL,DE
  // unsets
  cpu->f.set(FLAG_S);

  // sets half carry flag
  cpu->hl.set(0x0800);
  cpu->de.set(0x0801);
  cpu->op_19();
  CpuState expected_state = {.f = FLAG_H, .de = 0x0801, .hl = 0x1001};
  EXPECT_EQ(expected_state, cpu);

  // sets carry flag
  cpu->de.set(0xFFFF);
  cpu->op_19();
  expected_state = {.f = FLAG_C | FLAG_H, .de = 0xFFFF, .hl = 0x1000};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_1a) { // LD A,(DE)
  cpu->de.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(BYTE));

  cpu->op_1a();
  CpuState expected_state = {.a = BYTE, .de = ADDR};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_1b) { // DEC DE
  cpu->op_1b();

  // overflow
  CpuState expected_state = {.de = 0xFFFF};
  EXPECT_EQ(expected_state, cpu);

  // normal case
  cpu->op_1b();

  expected_state = {.de = 0xFFFE};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_1c) { // INC E
  // unsets
  cpu->f.set(FLAG_S);
  cpu->op_1c();

  CpuState expected_state = {.e = 1};
  EXPECT_EQ(expected_state, cpu);

  // half carry flag
  cpu->e.set(0x0F);
  cpu->op_1c();

  expected_state = {.f = FLAG_H, .e = 0x10};
  EXPECT_EQ(expected_state, cpu);

  // zero flag
  cpu->e.set(0xFF);
  cpu->op_1c();

  expected_state = {.f = FLAG_Z | FLAG_H, .e = 0};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_1d) { // DEC E
  cpu->op_1d();
  // subtract flag always set

  // half carry flag
  CpuState expected_state = {.f = FLAG_S | FLAG_H, .e = 0xFF};
  EXPECT_EQ(expected_state, cpu);

  // zero flag
  cpu->e.set(0x01);
  cpu->op_1d();

  expected_state = {.f = FLAG_Z | FLAG_S, .e = 0};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_1e) { // LD E,d8
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1)).WillOnce(Return(BYTE));

  cpu->op_1e();
  CpuState expected_state = {.e = BYTE};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_1f) { // RRA
  // unsets
  cpu->f.set(FLAG_Z | FLAG_S | FLAG_H);
  cpu->a.set(0b00000010);
  cpu->op_1f();

  CpuState expected_state = {.a = 0b00000001};
  EXPECT_EQ(expected_state, cpu);

  // set carry flag when shifting lsb
  cpu->op_1f();

  // zero flag isn't set, carry flag gets set
  expected_state = {.a = 0b00000000, .f = FLAG_C};
  EXPECT_EQ(expected_state, cpu);

  // carry flag carries over
  cpu->op_1f();
  expected_state = {.a = 0b10000000};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_20) { // JR NZ,r8
  // does not jump if zero flag is set
  cpu->f.set_zero_flag();
  cpu->op_20();
  CpuState expected_state = {.f = FLAG_Z, .pc = 0};
  EXPECT_EQ(expected_state, cpu);

  // jumps if zero flag is unset
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1)).WillOnce(Return(BYTE));
  cpu->f.clear_zero_flag();
  cpu->op_20();
  expected_state = {
      .f = 0, .pc = static_cast<word_t>(BYTE), .did_branch_ = true};
  EXPECT_EQ(expected_state, cpu);

  // treats the byte as a signed value
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1)).WillOnce(Return(~BYTE + 1));
  cpu->op_20();
  expected_state = {.did_branch_ = true};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_21) { // LD HL,d16
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1)).WillOnce(Return(0xFF & WORD));
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 2))
      .WillOnce(Return((WORD >> 8) & 0xFF));

  cpu->op_21();

  CpuState expected_state = {.hl = WORD};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_22) { // LDI (HL),A
  cpu->a.set(BYTE);
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, write(ADDR, BYTE)).Times(1);

  cpu->op_22();

  CpuState expected_state = {.a = BYTE, .hl = ADDR + 1};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_23) { // INC HL
  cpu->op_23();

  CpuState expected_state = {.hl = 1};
  EXPECT_EQ(expected_state, cpu);

  // overflow
  cpu->hl.set(0xFFFF);
  cpu->op_23();

  expected_state = {.hl = 0};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_24) { // INC H
  // unsets
  cpu->f.set(FLAG_S);
  cpu->op_24();

  CpuState expected_state = {.h = 1};
  EXPECT_EQ(expected_state, cpu);

  // half carry flag
  cpu->h.set(0x0F);
  cpu->op_24();

  expected_state = {.f = FLAG_H, .h = 0x10};
  EXPECT_EQ(expected_state, cpu);

  // zero flag
  cpu->h.set(0xFF);
  cpu->op_24();

  expected_state = {.f = FLAG_Z | FLAG_H, .h = 0};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_25) { // DEC H
  cpu->op_25();
  // subtract flag always set

  // half carry flag
  CpuState expected_state = {.f = FLAG_S | FLAG_H, .h = 0xFF};
  EXPECT_EQ(expected_state, cpu);

  // zero flag
  cpu->h.set(0x01);
  cpu->op_25();

  expected_state = {.f = FLAG_Z | FLAG_S, .h = 0};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_26) { // LD H,d8
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1)).WillOnce(Return(BYTE));

  cpu->op_26();

  CpuState expected_state = {.h = BYTE};
  EXPECT_EQ(expected_state, cpu);
}
/*
TEST_F(CpuTest, op_27) {
  // TODO
  EXPECT_TRUE(false);
}
*/
TEST_F(CpuTest, op_28) { // JR Z,r8
  // does not jump if zero flag is unset
  cpu->f.clear_zero_flag();
  cpu->op_28();
  CpuState expected_state = {.f = 0, .pc = 0, .did_branch_ = false};
  EXPECT_EQ(expected_state, cpu);

  // jumps if zero flag is unset
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1)).WillOnce(Return(BYTE));
  cpu->f.set_zero_flag();
  cpu->op_28();
  expected_state = {
      .f = FLAG_Z, .pc = static_cast<word_t>(BYTE), .did_branch_ = true};
  EXPECT_EQ(expected_state, cpu);

  // treats the byte as a signed value
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1)).WillOnce(Return(~BYTE + 1));
  cpu->op_28();
  expected_state = {.f = FLAG_Z, .did_branch_ = true};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_29) { // ADD HL,HL
  // unsets
  cpu->f.set(FLAG_S);

  // sets half carry flag
  cpu->hl.set(0x0801);
  cpu->op_29();
  CpuState expected_state = {.f = FLAG_H, .hl = 0x1002};
  EXPECT_EQ(expected_state, cpu);

  // sets carry flag
  cpu->hl.set(0x8001);
  cpu->op_29();
  expected_state = {.f = FLAG_C, .hl = 0x0002};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_2a) { // LDI A,(HL)
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(BYTE));

  cpu->op_2a();

  CpuState expected_state = {.a = BYTE, .hl = ADDR + 1};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_2b) { // DEC HL
  cpu->op_2b();

  // overflow
  CpuState expected_state = {.hl = 0xFFFF};
  EXPECT_EQ(expected_state, cpu);

  // normal case
  cpu->op_2b();

  expected_state = {.hl = 0xFFFE};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_2c) { // INC L
  // unsets
  cpu->f.set(FLAG_S);
  cpu->op_2c();

  CpuState expected_state = {.l = 1};
  EXPECT_EQ(expected_state, cpu);

  // half carry flag
  cpu->l.set(0x0F);
  cpu->op_2c();

  expected_state = {.f = FLAG_H, .l = 0x10};
  EXPECT_EQ(expected_state, cpu);

  // zero flag
  cpu->l.set(0xFF);
  cpu->op_2c();

  expected_state = {.f = FLAG_Z | FLAG_H, .l = 0};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_2d) { // DEC L
  cpu->op_2d();
  // subtract flag always set

  // half carry flag
  CpuState expected_state = {.f = FLAG_S | FLAG_H, .l = 0xFF};
  EXPECT_EQ(expected_state, cpu);

  // zero flag
  cpu->l.set(0x01);
  cpu->op_2d();

  expected_state = {.f = FLAG_Z | FLAG_S, .l = 0};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_2e) { // LD H,d8
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1)).WillOnce(Return(BYTE));

  cpu->op_2e();

  CpuState expected_state = {.l = BYTE};
  EXPECT_EQ(expected_state, cpu);
}
/*
TEST_F(CpuTest, op_2f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_30) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_31) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_32) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_33) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_34) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_35) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_36) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_37) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_38) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_39) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_3a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_3b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_3c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_3d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_3e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_3f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_40) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_41) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_42) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_43) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_44) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_45) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_46) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_47) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_48) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_49) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_4a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_4b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_4c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_4d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_4e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_4f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_50) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_51) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_52) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_53) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_54) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_55) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_56) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_57) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_58) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_59) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_5a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_5b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_5c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_5d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_5e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_5f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_60) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_61) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_62) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_63) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_64) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_65) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_66) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_67) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_68) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_69) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_6a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_6b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_6c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_6d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_6e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_6f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_70) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_71) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_72) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_73) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_74) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_75) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_76) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_77) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_78) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_79) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_7a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_7b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_7c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_7d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_7e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_7f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_80) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_81) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_82) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_83) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_84) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_85) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_86) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_87) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_88) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_89) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_8a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_8b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_8c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_8d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_8e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_8f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_90) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_91) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_92) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_93) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_94) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_95) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_96) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_97) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_98) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_99) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_9a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_9b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_9c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_9d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_9e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_9f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_a0) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_a1) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_a2) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_a3) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_a4) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_a5) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_a6) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_a7) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_a8) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_a9) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_aa) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_ab) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_ac) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_ad) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_ae) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_af) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_b0) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_b1) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_b2) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_b3) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_b4) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_b5) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_b6) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_b7) {
  // TODO
  EXPECT_TRUE(false);
}
*/
TEST_F(CpuTest, op_b8) {
  cpu->a.set(0x3C);
  cpu->b.set(0x2F);
  cpu->op_b8();
  // sets carry flag
  CpuState expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_H, .b = 0x2F };
  EXPECT_EQ(expected_state, cpu);

  // sets half carry
  cpu->b.set(0x40);
  cpu->op_b8();
  expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_C, .b = 0x40 };
  EXPECT_EQ(expected_state, cpu);

  // sets zero carry
  cpu->b.set(0x3C);
  cpu->op_b8();
  expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_Z, .b = 0x3C };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_b9) {
  cpu->a.set(0x3C);
  cpu->c.set(0x2F);
  cpu->op_b9();
  // sets carry flag
  CpuState expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_H, .c = 0x2F };
  EXPECT_EQ(expected_state, cpu);

  // sets half carry
  cpu->c.set(0x40);
  cpu->op_b9();
  expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_C, .c = 0x40 };
  EXPECT_EQ(expected_state, cpu);

  // sets zero carry
  cpu->c.set(0x3C);
  cpu->op_b9();
  expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_Z, .c = 0x3C };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_ba) {
  cpu->a.set(0x3C);
  cpu->d.set(0x2F);
  cpu->op_ba();
  // sets carry flag
  CpuState expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_H, .d = 0x2F };
  EXPECT_EQ(expected_state, cpu);

  // sets half carry
  cpu->d.set(0x40);
  cpu->op_ba();
  expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_C, .d = 0x40 };
  EXPECT_EQ(expected_state, cpu);

  // sets zero carry
  cpu->d.set(0x3C);
  cpu->op_ba();
  expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_Z, .d = 0x3C };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_bb) {
  cpu->a.set(0x3C);
  cpu->e.set(0x2F);
  cpu->op_bb();
  // sets carry flag
  CpuState expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_H, .e = 0x2F };
  EXPECT_EQ(expected_state, cpu);

  // sets half carry
  cpu->e.set(0x40);
  cpu->op_bb();
  expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_C, .e = 0x40 };
  EXPECT_EQ(expected_state, cpu);

  // sets zero carry
  cpu->e.set(0x3C);
  cpu->op_bb();
  expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_Z, .e = 0x3C };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_bc) {
  cpu->a.set(0x3C);
  cpu->h.set(0x2F);
  cpu->op_bc();
  // sets carry flag
  CpuState expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_H, .h = 0x2F };
  EXPECT_EQ(expected_state, cpu);

  // sets half carry
  cpu->h.set(0x40);
  cpu->op_bc();
  expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_C, .h = 0x40 };
  EXPECT_EQ(expected_state, cpu);

  // sets zero carry
  cpu->h.set(0x3C);
  cpu->op_bc();
  expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_Z, .h = 0x3C };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_bd) {
  cpu->a.set(0x3C);
  cpu->l.set(0x2F);
  cpu->op_bd();
  // sets carry flag
  CpuState expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_H, .l = 0x2F };
  EXPECT_EQ(expected_state, cpu);

  // sets half carry
  cpu->l.set(0x40);
  cpu->op_bd();
  expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_C, .l = 0x40 };
  EXPECT_EQ(expected_state, cpu);

  // sets zero carry
  cpu->l.set(0x3C);
  cpu->op_bd();
  expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_Z, .l = 0x3C };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_be) {
  cpu->a.set(0x3C);
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(0x2F));
  cpu->op_be();
  // sets carry flag
  CpuState expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_H, .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);

  // sets half carry
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(0x40));
  cpu->op_be();
  expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_C, .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);

  // sets zero carry
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(0x3C));
  cpu->op_be();
  expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_Z, .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_bf) {
  // I think this function only sets the subtract/zero flags and that's it
  cpu->a.set(0xFF);
  cpu->op_bf();
  CpuState expected_state = {.a = 0xFF, .f = FLAG_S | FLAG_Z};
  EXPECT_EQ(expected_state, cpu);
}
/*
TEST_F(CpuTest, op_c0) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_c1) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_c2) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_c3) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_c4) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_c5) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_c6) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_c7) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_c8) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_c9) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_ca) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cc) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cd) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_ce) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cf) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_d0) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_d1) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_d2) {
  // TODO
  EXPECT_TRUE(false);
}
*/
TEST_F(CpuTest, op_d3) {
  EXPECT_ANY_THROW(cpu->op_d3());
}
/*
TEST_F(CpuTest, op_d4) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_d5) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_d6) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_d7) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_d8) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_d9) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_da) {
  // TODO
  EXPECT_TRUE(false);
}
*/
TEST_F(CpuTest, op_db) {
  EXPECT_ANY_THROW(cpu->op_db());
}
/*
TEST_F(CpuTest, op_dc) {
  // TODO
  EXPECT_TRUE(false);
}
*/
TEST_F(CpuTest, op_dd) {
  EXPECT_ANY_THROW(cpu->op_dd());
}
/*
TEST_F(CpuTest, op_de) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_df) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_e0) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_e1) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_e2) {
  // TODO
  EXPECT_TRUE(false);
}
*/
TEST_F(CpuTest, op_e3) {
  EXPECT_ANY_THROW(cpu->op_e3());
}

TEST_F(CpuTest, op_e4) {
  EXPECT_ANY_THROW(cpu->op_e4());
}
/*
TEST_F(CpuTest, op_e5) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_e6) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_e7) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_e8) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_e9) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_ea) {
  // TODO
  EXPECT_TRUE(false);
}
*/
TEST_F(CpuTest, op_eb) {
  EXPECT_ANY_THROW(cpu->op_eb());
}

TEST_F(CpuTest, op_ec) {
  EXPECT_ANY_THROW(cpu->op_ec());
}

TEST_F(CpuTest, op_ed) {
  EXPECT_ANY_THROW(cpu->op_ed());
}
/*
TEST_F(CpuTest, op_ee) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_ef) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_f0) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_f1) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_f2) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_f3) {
  // TODO
  EXPECT_TRUE(false);
}
*/
TEST_F(CpuTest, op_f4) {
  EXPECT_ANY_THROW(cpu->op_f4());
}
/*
TEST_F(CpuTest, op_f5) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_f6) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_f7) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_f8) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_f9) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_fa) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_fb) {
  // TODO
  EXPECT_TRUE(false);
}
*/
TEST_F(CpuTest, op_fc) {
  EXPECT_ANY_THROW(cpu->op_fc());
}

TEST_F(CpuTest, op_fd) {
  EXPECT_ANY_THROW(cpu->op_fd());
}

TEST_F(CpuTest, op_fe) {
  cpu->a.set(0x3C);
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1)).WillOnce(Return(0x2F));
  cpu->op_fe();
  // sets carry flag
  CpuState expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_H};
  EXPECT_EQ(expected_state, cpu);

  // sets half carry
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1)).WillOnce(Return(0x40));
  cpu->op_fe();
  expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_C};
  EXPECT_EQ(expected_state, cpu);

  // sets zero carry
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1)).WillOnce(Return(0x3C));
  cpu->op_fe();
  expected_state = {.a = 0x3C, .f = FLAG_S | FLAG_Z};
  EXPECT_EQ(expected_state, cpu);
}
/*
TEST_F(CpuTest, op_ff) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_00) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_01) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_02) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_03) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_04) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_05) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_06) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_07) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_08) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_09) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_0a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_0b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_0c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_0d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_0e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_0f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_10) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_11) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_12) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_13) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_14) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_15) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_16) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_17) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_18) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_19) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_1a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_1b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_1c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_1d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_1e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_1f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_20) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_21) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_22) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_23) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_24) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_25) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_26) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_27) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_28) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_29) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_2a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_2b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_2c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_2d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_2e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_2f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_30) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_31) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_32) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_33) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_34) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_35) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_36) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_37) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_38) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_39) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_3a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_3b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_3c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_3d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_3e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_3f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_40) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_41) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_42) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_43) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_44) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_45) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_46) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_47) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_48) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_49) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_4a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_4b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_4c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_4d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_4e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_4f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_50) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_51) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_52) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_53) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_54) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_55) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_56) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_57) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_58) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_59) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_5a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_5b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_5c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_5d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_5e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_5f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_60) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_61) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_62) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_63) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_64) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_65) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_66) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_67) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_68) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_69) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_6a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_6b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_6c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_6d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_6e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_6f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_70) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_71) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_72) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_73) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_74) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_75) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_76) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_77) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_78) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_79) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_7a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_7b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_7c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_7d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_7e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_7f) {
  // TODO
  EXPECT_TRUE(false);
}
*/
TEST_F(CpuTest, op_cb_80) {
  cpu->b.set_bit(0);
  cpu->op_cb_80();

  CpuState expected_state = { .b = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_81) {
  cpu->c.set_bit(0);
  cpu->op_cb_81();

  CpuState expected_state = { .c = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_82) {
  cpu->d.set_bit(0);
  cpu->op_cb_82();

  CpuState expected_state = { .d = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_83) {
  cpu->e.set_bit(0);
  cpu->op_cb_83();

  CpuState expected_state = { .e = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_84) {
  cpu->h.set_bit(0);
  cpu->op_cb_84();

  CpuState expected_state = { .h = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_85) {
  cpu->l.set_bit(0);
  cpu->op_cb_85();

  CpuState expected_state = { .l = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_86) {
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(1 << 0));
  EXPECT_CALL(*mmu, write(ADDR, 0)).Times(1);

  cpu->op_cb_86();
  CpuState expected_state = { .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_87) {
  cpu->a.set_bit(0);
  cpu->op_cb_87();

  CpuState expected_state = { .a = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_88) {
  cpu->b.set_bit(1);
  cpu->op_cb_88();

  CpuState expected_state = { .b = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_89) {
  cpu->c.set_bit(1);
  cpu->op_cb_89();

  CpuState expected_state = { .c = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_8a) {
  cpu->d.set_bit(1);
  cpu->op_cb_8a();

  CpuState expected_state = { .d = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_8b) {
  cpu->e.set_bit(1);
  cpu->op_cb_8b();

  CpuState expected_state = { .e = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_8c) {
  cpu->h.set_bit(1);
  cpu->op_cb_8c();

  CpuState expected_state = { .h = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_8d) {
  cpu->l.set_bit(1);
  cpu->op_cb_8d();

  CpuState expected_state = { .l = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_8e) {
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(1 << 1));
  EXPECT_CALL(*mmu, write(ADDR, 0)).Times(1);

  cpu->op_cb_8e();
  CpuState expected_state = { .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_8f) {
  cpu->a.set_bit(1);
  cpu->op_cb_8f();

  CpuState expected_state = { .a = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_90) {
  cpu->b.set_bit(2);
  cpu->op_cb_90();

  CpuState expected_state = { .b = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_91) {
  cpu->c.set_bit(2);
  cpu->op_cb_91();

  CpuState expected_state = { .c = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_92) {
  cpu->d.set_bit(2);
  cpu->op_cb_92();

  CpuState expected_state = { .d = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_93) {
  cpu->e.set_bit(2);
  cpu->op_cb_93();

  CpuState expected_state = { .e = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_94) {
  cpu->h.set_bit(2);
  cpu->op_cb_94();

  CpuState expected_state = { .h = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_95) {
  cpu->l.set_bit(2);
  cpu->op_cb_95();

  CpuState expected_state = { .l = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_96) {
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(1 << 2));
  EXPECT_CALL(*mmu, write(ADDR, 0)).Times(1);

  cpu->op_cb_96();
  CpuState expected_state = { .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_97) {
  cpu->a.set_bit(2);
  cpu->op_cb_97();

  CpuState expected_state = { .a = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_98) {
  cpu->b.set_bit(3);
  cpu->op_cb_98();

  CpuState expected_state = { .b = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_99) {
  cpu->c.set_bit(3);
  cpu->op_cb_99();

  CpuState expected_state = { .c = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_9a) {
  cpu->d.set_bit(3);
  cpu->op_cb_9a();

  CpuState expected_state = { .d = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_9b) {
  cpu->e.set_bit(3);
  cpu->op_cb_9b();

  CpuState expected_state = { .e = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_9c) {
  cpu->h.set_bit(3);
  cpu->op_cb_9c();

  CpuState expected_state = { .h = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_9d) {
  cpu->l.set_bit(3);
  cpu->op_cb_9d();

  CpuState expected_state = { .l = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_9e) {
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(1 << 3));
  EXPECT_CALL(*mmu, write(ADDR, 0)).Times(1);

  cpu->op_cb_9e();
  CpuState expected_state = { .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_9f) {
  cpu->a.set_bit(3);
  cpu->op_cb_9f();

  CpuState expected_state = { .a = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_a0) {
  cpu->b.set_bit(4);
  cpu->op_cb_a0();

  CpuState expected_state = { .b = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_a1) {
  cpu->c.set_bit(4);
  cpu->op_cb_a1();

  CpuState expected_state = { .c = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_a2) {
  cpu->d.set_bit(4);
  cpu->op_cb_a2();

  CpuState expected_state = { .d = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_a3) {
  cpu->e.set_bit(4);
  cpu->op_cb_a3();

  CpuState expected_state = { .e = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_a4) {
  cpu->h.set_bit(4);
  cpu->op_cb_a4();

  CpuState expected_state = { .h = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_a5) {
  cpu->l.set_bit(4);
  cpu->op_cb_a5();

  CpuState expected_state = { .l = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_a6) {
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(1 << 4));
  EXPECT_CALL(*mmu, write(ADDR, 0)).Times(1);

  cpu->op_cb_a6();
  CpuState expected_state = { .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_a7) {
  cpu->a.set_bit(4);
  cpu->op_cb_a7();

  CpuState expected_state = { .a = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_a8) {
  cpu->b.set_bit(5);
  cpu->op_cb_a8();

  CpuState expected_state = { .b = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_a9) {
  cpu->c.set_bit(5);
  cpu->op_cb_a9();

  CpuState expected_state = { .c = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_aa) {
  cpu->d.set_bit(5);
  cpu->op_cb_aa();

  CpuState expected_state = { .d = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_ab) {
  cpu->e.set_bit(5);
  cpu->op_cb_ab();

  CpuState expected_state = { .e = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_ac) {
  cpu->h.set_bit(5);
  cpu->op_cb_ac();

  CpuState expected_state = { .h = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_ad) {
  cpu->l.set_bit(5);
  cpu->op_cb_ad();

  CpuState expected_state = { .l = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_ae) {
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(1 << 5));
  EXPECT_CALL(*mmu, write(ADDR, 0)).Times(1);

  cpu->op_cb_ae();
  CpuState expected_state = { .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_af) {
  cpu->a.set_bit(5);
  cpu->op_cb_af();

  CpuState expected_state = { .a = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_b0) {
  cpu->b.set_bit(6);
  cpu->op_cb_b0();

  CpuState expected_state = { .b = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_b1) {
  cpu->c.set_bit(6);
  cpu->op_cb_b1();

  CpuState expected_state = { .c = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_b2) {
  cpu->d.set_bit(6);
  cpu->op_cb_b2();

  CpuState expected_state = { .d = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_b3) {
  cpu->e.set_bit(6);
  cpu->op_cb_b3();

  CpuState expected_state = { .e = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_b4) {
  cpu->h.set_bit(6);
  cpu->op_cb_b4();

  CpuState expected_state = { .h = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_b5) {
  cpu->l.set_bit(6);
  cpu->op_cb_b5();

  CpuState expected_state = { .l = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_b6) {
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(1 << 6));
  EXPECT_CALL(*mmu, write(ADDR, 0)).Times(1);

  cpu->op_cb_b6();
  CpuState expected_state = { .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_b7) {
  cpu->a.set_bit(6);
  cpu->op_cb_b7();

  CpuState expected_state = { .a = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_b8) {
  cpu->b.set_bit(7);
  cpu->op_cb_b8();

  CpuState expected_state = { .b = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_b9) {
  cpu->c.set_bit(7);
  cpu->op_cb_b9();

  CpuState expected_state = { .c = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_ba) {
  cpu->d.set_bit(7);
  cpu->op_cb_ba();

  CpuState expected_state = { .d = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_bb) {
  cpu->e.set_bit(7);
  cpu->op_cb_bb();

  CpuState expected_state = { .e = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_bc) {
  cpu->h.set_bit(7);
  cpu->op_cb_bc();

  CpuState expected_state = { .h = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_bd) {
  cpu->l.set_bit(7);
  cpu->op_cb_bd();

  CpuState expected_state = { .l = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_be) {
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(1 << 7));
  EXPECT_CALL(*mmu, write(ADDR, 0)).Times(1);

  cpu->op_cb_be();
  CpuState expected_state = { .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_bf) {
  cpu->a.set_bit(7);
  cpu->op_cb_bf();

  CpuState expected_state = { .a = 0 };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_c0) {
  cpu->op_cb_c0();

  CpuState expected_state = { .b = (1 << 0) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_c1) {
  cpu->op_cb_c1();

  CpuState expected_state = { .c = (1 << 0) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_c2) {
  cpu->op_cb_c2();

  CpuState expected_state = { .d = (1 << 0) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_c3) {
  cpu->op_cb_c3();

  CpuState expected_state = { .e = (1 << 0) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_c4) {
  cpu->op_cb_c4();

  CpuState expected_state = { .h = (1 << 0) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_c5) {
  cpu->op_cb_c5();

  CpuState expected_state = { .l = (1 << 0) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_c6) {
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(0));
  EXPECT_CALL(*mmu, write(ADDR, (1 << 0))).Times(1);

  cpu->op_cb_c6();
  CpuState expected_state = { .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_c7) {
  cpu->op_cb_c7();

  CpuState expected_state = { .a = (1 << 0) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_c8) {
  cpu->op_cb_c8();

  CpuState expected_state = { .b = (1 << 1) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_c9) {
  cpu->op_cb_c9();

  CpuState expected_state = { .c = (1 << 1) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_ca) {
  cpu->op_cb_ca();

  CpuState expected_state = { .d = (1 << 1) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_cb) {
  cpu->op_cb_cb();

  CpuState expected_state = { .e = (1 << 1) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_cc) {
  cpu->op_cb_cc();

  CpuState expected_state = { .h = (1 << 1) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_cd) {
  cpu->op_cb_cd();

  CpuState expected_state = { .l = (1 << 1) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_ce) {
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(0));
  EXPECT_CALL(*mmu, write(ADDR, (1 << 1))).Times(1);

  cpu->op_cb_ce();
  CpuState expected_state = { .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_cf) {
  cpu->op_cb_cf();

  CpuState expected_state = { .a = (1 << 1) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_d0) {
  cpu->op_cb_d0();

  CpuState expected_state = { .b = (1 << 2) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_d1) {
  cpu->op_cb_d1();

  CpuState expected_state = { .c = (1 << 2) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_d2) {
  cpu->op_cb_d2();

  CpuState expected_state = { .d = (1 << 2) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_d3) {
  cpu->op_cb_d3();

  CpuState expected_state = { .e = (1 << 2) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_d4) {
  cpu->op_cb_d4();

  CpuState expected_state = { .h = (1 << 2) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_d5) {
  cpu->op_cb_d5();

  CpuState expected_state = { .l = (1 << 2) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_d6) {
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(0));
  EXPECT_CALL(*mmu, write(ADDR, (1 << 2))).Times(1);

  cpu->op_cb_d6();
  CpuState expected_state = { .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_d7) {
  cpu->op_cb_d7();

  CpuState expected_state = { .a = (1 << 2) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_d8) {
  cpu->op_cb_d8();

  CpuState expected_state = { .b = (1 << 3) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_d9) {
  cpu->op_cb_d9();

  CpuState expected_state = { .c = (1 << 3) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_da) {
  cpu->op_cb_da();

  CpuState expected_state = { .d = (1 << 3) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_db) {
  cpu->op_cb_db();

  CpuState expected_state = { .e = (1 << 3) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_dc) {
  cpu->op_cb_dc();

  CpuState expected_state = { .h = (1 << 3) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_dd) {
  cpu->op_cb_dd();

  CpuState expected_state = { .l = (1 << 3) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_de) {
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(0));
  EXPECT_CALL(*mmu, write(ADDR, (1 << 3))).Times(1);

  cpu->op_cb_de();
  CpuState expected_state = { .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_df) {
  cpu->op_cb_df();

  CpuState expected_state = { .a = (1 << 3) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_e0) {
  cpu->op_cb_e0();

  CpuState expected_state = { .b = (1 << 4) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_e1) {
  cpu->op_cb_e1();

  CpuState expected_state = { .c = (1 << 4) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_e2) {
  cpu->op_cb_e2();

  CpuState expected_state = { .d = (1 << 4) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_e3) {
  cpu->op_cb_e3();

  CpuState expected_state = { .e = (1 << 4) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_e4) {
  cpu->op_cb_e4();

  CpuState expected_state = { .h = (1 << 4) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_e5) {
  cpu->op_cb_e5();

  CpuState expected_state = { .l = (1 << 4) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_e6) {
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(0));
  EXPECT_CALL(*mmu, write(ADDR, (1 << 4))).Times(1);

  cpu->op_cb_e6();
  CpuState expected_state = { .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_e7) {
  cpu->op_cb_e7();

  CpuState expected_state = { .a = (1 << 4) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_e8) {
  cpu->op_cb_e8();

  CpuState expected_state = { .b = (1 << 5) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_e9) {
  cpu->op_cb_e9();

  CpuState expected_state = { .c = (1 << 5) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_ea) {
  cpu->op_cb_ea();

  CpuState expected_state = { .d = (1 << 5) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_eb) {
  cpu->op_cb_eb();

  CpuState expected_state = { .e = (1 << 5) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_ec) {
  cpu->op_cb_ec();

  CpuState expected_state = { .h = (1 << 5) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_ed) {
  cpu->op_cb_ed();

  CpuState expected_state = { .l = (1 << 5) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_ee) {
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(0));
  EXPECT_CALL(*mmu, write(ADDR, (1 << 5))).Times(1);

  cpu->op_cb_ee();
  CpuState expected_state = { .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_ef) {
  cpu->op_cb_ef();

  CpuState expected_state = { .a = (1 << 5) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_f0) {
  cpu->op_cb_f0();

  CpuState expected_state = { .b = (1 << 6) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_f1) {
  cpu->op_cb_f1();

  CpuState expected_state = { .c = (1 << 6) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_f2) {
  cpu->op_cb_f2();

  CpuState expected_state = { .d = (1 << 6) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_f3) {
  cpu->op_cb_f3();

  CpuState expected_state = { .e = (1 << 6) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_f4) {
  cpu->op_cb_f4();

  CpuState expected_state = { .h = (1 << 6) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_f5) {
  cpu->op_cb_f5();

  CpuState expected_state = { .l = (1 << 6) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_f6) {
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(0));
  EXPECT_CALL(*mmu, write(ADDR, (1 << 6))).Times(1);

  cpu->op_cb_f6();
  CpuState expected_state = { .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_f7) {
  cpu->op_cb_f7();

  CpuState expected_state = { .a = (1 << 6) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_f8) {
  cpu->op_cb_f8();

  CpuState expected_state = { .b = (1 << 7) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_f9) {
  cpu->op_cb_f9();

  CpuState expected_state = { .c = (1 << 7) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_fa) {
  cpu->op_cb_fa();

  CpuState expected_state = { .d = (1 << 7) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_fb) {
  cpu->op_cb_fb();

  CpuState expected_state = { .e = (1 << 7) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_fc) {
  cpu->op_cb_fc();

  CpuState expected_state = { .h = (1 << 7) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_fd) {
  cpu->op_cb_fd();

  CpuState expected_state = { .l = (1 << 7) };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_fe) {
  cpu->hl.set(ADDR);
  EXPECT_CALL(*mmu, read(ADDR)).WillOnce(Return(0));
  EXPECT_CALL(*mmu, write(ADDR, (1 << 7))).Times(1);

  cpu->op_cb_fe();
  CpuState expected_state = { .hl = ADDR };
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_cb_ff) {
  cpu->op_cb_ff();

  CpuState expected_state = { .a = (1 << 7) };
  EXPECT_EQ(expected_state, cpu);
}

} // namespace gbc
