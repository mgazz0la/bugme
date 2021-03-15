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
      .Times(1)
      .WillOnce(Return(0xFF & WORD));
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 2))
      .Times(1)
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
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1)).Times(1).WillOnce(Return(BYTE));

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
      .Times(1)
      .WillOnce(Return(0xFF & WORD));
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 2))
      .Times(1)
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
  EXPECT_CALL(*mmu, read(ADDR)).Times(1).WillOnce(Return(BYTE));

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
  cpu->f.set(FLAG_S);    // unsets
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
  EXPECT_CALL(*mmu, read(cpu->pc.value() + 1))
      .Times(1)
      .WillOnce(Return(BYTE));

  cpu->op_0e();
  CpuState expected_state = {.c = BYTE};
  EXPECT_EQ(expected_state, cpu);
}

TEST_F(CpuTest, op_0f) { // RRC A
  cpu->f.set(FLAG_Z | FLAG_S | FLAG_H); // unsets
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
/*
TEST_F(CpuTest, op_10) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_11) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_12) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_13) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_14) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_15) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_16) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_17) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_18) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_19) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_1a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_1b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_1c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_1d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_1e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_1f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_20) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_21) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_22) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_23) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_24) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_25) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_26) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_27) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_28) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_29) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_2a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_2b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_2c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_2d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_2e) {
  // TODO
  EXPECT_TRUE(false);
}

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

TEST_F(CpuTest, op_b8) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_b9) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_ba) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_bb) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_bc) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_bd) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_be) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_bf) {
  // TODO
  EXPECT_TRUE(false);
}

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

TEST_F(CpuTest, op_d3) {
  // TODO
  EXPECT_TRUE(false);
}

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

TEST_F(CpuTest, op_db) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_dc) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_dd) {
  // TODO
  EXPECT_TRUE(false);
}

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

TEST_F(CpuTest, op_e3) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_e4) {
  // TODO
  EXPECT_TRUE(false);
}

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

TEST_F(CpuTest, op_eb) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_ec) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_ed) {
  // TODO
  EXPECT_TRUE(false);
}

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

TEST_F(CpuTest, op_f4) {
  // TODO
  EXPECT_TRUE(false);
}

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

TEST_F(CpuTest, op_fc) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_fd) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_fe) {
  // TODO
  EXPECT_TRUE(false);
}

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

TEST_F(CpuTest, op_cb_80) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_81) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_82) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_83) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_84) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_85) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_86) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_87) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_88) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_89) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_8a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_8b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_8c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_8d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_8e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_8f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_90) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_91) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_92) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_93) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_94) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_95) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_96) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_97) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_98) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_99) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_9a) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_9b) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_9c) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_9d) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_9e) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_9f) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_a0) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_a1) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_a2) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_a3) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_a4) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_a5) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_a6) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_a7) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_a8) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_a9) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_aa) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_ab) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_ac) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_ad) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_ae) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_af) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_b0) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_b1) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_b2) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_b3) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_b4) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_b5) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_b6) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_b7) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_b8) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_b9) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_ba) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_bb) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_bc) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_bd) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_be) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_bf) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_c0) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_c1) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_c2) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_c3) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_c4) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_c5) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_c6) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_c7) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_c8) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_c9) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_ca) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_cb) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_cc) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_cd) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_ce) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_cf) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_d0) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_d1) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_d2) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_d3) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_d4) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_d5) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_d6) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_d7) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_d8) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_d9) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_da) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_db) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_dc) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_dd) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_de) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_df) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_e0) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_e1) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_e2) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_e3) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_e4) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_e5) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_e6) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_e7) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_e8) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_e9) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_ea) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_eb) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_ec) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_ed) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_ee) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_ef) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_f0) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_f1) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_f2) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_f3) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_f4) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_f5) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_f6) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_f7) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_f8) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_f9) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_fa) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_fb) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_fc) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_fd) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_fe) {
  // TODO
  EXPECT_TRUE(false);
}

TEST_F(CpuTest, op_cb_ff) {
  // TODO
  EXPECT_TRUE(false);
}
*/

} // namespace gbc
