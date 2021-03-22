#ifndef GBC_CPU_H
#define GBC_CPU_H

#include "register.hh"
#include <functional>
#include <map>
#include <memory>

namespace gbc {
namespace interrupt {
static const word_t VBLANK = 0x0040;
static const word_t LCDC_STATUS = 0x0048;
static const word_t TIMER = 0x0050;
static const word_t SERIAL = 0x0058;
static const word_t JOYPAD = 0x0060;
} // namespace interrupt

class Mmu;

class Cpu {
public:
  explicit Cpu(std::shared_ptr<Mmu> mmu);

  cycles_t tick();
  void reset();

  std::function<void()> vblank_cb();
  std::function<void()> lcdc_cb();
  std::function<void()> timer_cb();
  std::function<void()> serial_cb();
  std::function<void()> joypad_cb();
private:
  void check_interrupts();

  // Alias for reg.value()
  static inline word_t _(WordRegister &reg) { return reg.value(); }
  static inline byte_t _(ByteRegister &reg) { return reg.value(); }

  byte_t next_byte();
  word_t next_word();
  inline word_t a16() { return next_word(); }
  inline word_t d16() { return next_word(); }
  inline byte_t d8() { return next_byte(); }

  // "microcode" functions
  void nop() const;

  void ld(ByteRegister &reg);
  void ld(ByteRegister &reg, const word_t addr);
  void ld(ByteRegister &reg, const ByteRegister &other);
  void ld(WordRegister &reg, const word_t value);
  void ld(const word_t addr);
  void ld(const word_t addr, ByteRegister &reg);
  void ld(const word_t addr, WordRegister &reg);

  void ldi(const word_t addr, ByteRegister &reg);
  void ldi(ByteRegister &reg, const word_t addr);

  void ldd(const word_t addr, ByteRegister &reg);
  void ldd(ByteRegister &reg, const word_t addr);

  void inc(ByteRegister &reg);
  void inc(WordRegister &reg);
  void inc(const word_t addr);

  void dec(ByteRegister &reg);
  void dec(WordRegister &reg);
  void dec(const word_t addr);

  void rlc(ByteRegister &reg);
  void rlc(const byte_t addr);

  void rl(ByteRegister &reg);
  void rl(const byte_t addr);

  void rrc(ByteRegister &reg);
  void rrc(const byte_t addr);

  void rr(ByteRegister &reg);
  void rr(const byte_t addr);

  void add(ByteRegister &reg, const ByteRegister &other);
  void add(ByteRegister &reg, const word_t addr);
  void add(ByteRegister &reg);
  void add(WordRegister &reg, const WordRegister &other);

  void adc(ByteRegister &reg, const ByteRegister &other);
  void adc(ByteRegister &reg, const word_t addr);
  void adc(ByteRegister &reg);

  void sub(ByteRegister &reg, const ByteRegister &other);
  void sub(ByteRegister &reg, const word_t addr);
  void sub(ByteRegister &reg);

  void sbc(ByteRegister &reg, const ByteRegister &other);
  void sbc(ByteRegister &reg, const word_t addr);
  void sbc(ByteRegister &reg);

  void stop();

  void halt();

  void jr();

  void jr_if(bool condition);

  void a_and(const ByteRegister &other);
  void a_and(const word_t addr);
  void a_and();

  void a_or(const ByteRegister &other);
  void a_or(const word_t addr);
  void a_or();

  void a_xor(const ByteRegister &other);
  void a_xor(const word_t addr);
  void a_xor();

  void sla(ByteRegister &reg);
  void sla(const word_t addr);

  void sra(ByteRegister &reg);
  void sra(const word_t addr);

  void srl(ByteRegister &reg);
  void srl(const word_t addr);

  void swap(ByteRegister &reg);
  void swap(const word_t addr);

  void bit(const bit_t bit, const ByteRegister &reg);
  void bit(const bit_t bit, const word_t addr);

  void cp(ByteRegister &reg);
  void cp(const word_t addr);
  void cp();

  void res(const bit_t bit, ByteRegister &reg);
  void res(const bit_t bit, const word_t addr);

  void set(const bit_t bit, ByteRegister &reg);
  void set(const bit_t bit, const word_t addr);

  void pop(WordRegister &reg);

  void push(const WordRegister &reg);

  void ret();
  void ret_if(bool condition);
  void reti();

  void ldh(const byte_t addr_low, const ByteRegister &reg);
  void ldh(ByteRegister &reg, const byte_t addr_low);

  void call();
  void call_if(bool condition);

  void jp();
  void jp(const word_t addr);
  void jp_if(bool condition);

  void ei();
  void di();

  ByteRegister a, b, c, d, e, h, l;
  ByteRegisterPair af, bc, de, hl;
  WordValuedRegister pc;
  WordValuedRegister sp;
  FlagRegister f;

  std::shared_ptr<Mmu> mmu_;

  bool interrupt_master_enable;
  AddressRegister interrupt_enable;
  AddressRegister interrupt_flag;

  bool stopped_ = false;
  bool halted_ = false;
  bool did_branch_ = false;

  void op(word_t word);
  void cb_op(word_t word);

  /* clang-format off */
  void op_00(); void op_01(); void op_02(); void op_03(); void op_04(); void op_05(); void op_06(); void op_07(); void op_08(); void op_09(); void op_0a(); void op_0b(); void op_0c(); void op_0d(); void op_0e(); void op_0f();
  void op_10(); void op_11(); void op_12(); void op_13(); void op_14(); void op_15(); void op_16(); void op_17(); void op_18(); void op_19(); void op_1a(); void op_1b(); void op_1c(); void op_1d(); void op_1e(); void op_1f();
  void op_20(); void op_21(); void op_22(); void op_23(); void op_24(); void op_25(); void op_26(); void op_27(); void op_28(); void op_29(); void op_2a(); void op_2b(); void op_2c(); void op_2d(); void op_2e(); void op_2f();
  void op_30(); void op_31(); void op_32(); void op_33(); void op_34(); void op_35(); void op_36(); void op_37(); void op_38(); void op_39(); void op_3a(); void op_3b(); void op_3c(); void op_3d(); void op_3e(); void op_3f();
  void op_40(); void op_41(); void op_42(); void op_43(); void op_44(); void op_45(); void op_46(); void op_47(); void op_48(); void op_49(); void op_4a(); void op_4b(); void op_4c(); void op_4d(); void op_4e(); void op_4f();
  void op_50(); void op_51(); void op_52(); void op_53(); void op_54(); void op_55(); void op_56(); void op_57(); void op_58(); void op_59(); void op_5a(); void op_5b(); void op_5c(); void op_5d(); void op_5e(); void op_5f();
  void op_60(); void op_61(); void op_62(); void op_63(); void op_64(); void op_65(); void op_66(); void op_67(); void op_68(); void op_69(); void op_6a(); void op_6b(); void op_6c(); void op_6d(); void op_6e(); void op_6f();
  void op_70(); void op_71(); void op_72(); void op_73(); void op_74(); void op_75(); void op_76(); void op_77(); void op_78(); void op_79(); void op_7a(); void op_7b(); void op_7c(); void op_7d(); void op_7e(); void op_7f();
  void op_80(); void op_81(); void op_82(); void op_83(); void op_84(); void op_85(); void op_86(); void op_87(); void op_88(); void op_89(); void op_8a(); void op_8b(); void op_8c(); void op_8d(); void op_8e(); void op_8f();
  void op_90(); void op_91(); void op_92(); void op_93(); void op_94(); void op_95(); void op_96(); void op_97(); void op_98(); void op_99(); void op_9a(); void op_9b(); void op_9c(); void op_9d(); void op_9e(); void op_9f();
  void op_a0(); void op_a1(); void op_a2(); void op_a3(); void op_a4(); void op_a5(); void op_a6(); void op_a7(); void op_a8(); void op_a9(); void op_aa(); void op_ab(); void op_ac(); void op_ad(); void op_ae(); void op_af();
  void op_b0(); void op_b1(); void op_b2(); void op_b3(); void op_b4(); void op_b5(); void op_b6(); void op_b7(); void op_b8(); void op_b9(); void op_ba(); void op_bb(); void op_bc(); void op_bd(); void op_be(); void op_bf();
  void op_c0(); void op_c1(); void op_c2(); void op_c3(); void op_c4(); void op_c5(); void op_c6(); void op_c7(); void op_c8(); void op_c9(); void op_ca(); void op_cb(); void op_cc(); void op_cd(); void op_ce(); void op_cf();
  void op_d0(); void op_d1(); void op_d2(); void op_d3(); void op_d4(); void op_d5(); void op_d6(); void op_d7(); void op_d8(); void op_d9(); void op_da(); void op_db(); void op_dc(); void op_dd(); void op_de(); void op_df();
  void op_e0(); void op_e1(); void op_e2(); void op_e3(); void op_e4(); void op_e5(); void op_e6(); void op_e7(); void op_e8(); void op_e9(); void op_ea(); void op_eb(); void op_ec(); void op_ed(); void op_ee(); void op_ef();
  void op_f0(); void op_f1(); void op_f2(); void op_f3(); void op_f4(); void op_f5(); void op_f6(); void op_f7(); void op_f8(); void op_f9(); void op_fa(); void op_fb(); void op_fc(); void op_fd(); void op_fe(); void op_ff();

  void op_cb_00(); void op_cb_01(); void op_cb_02(); void op_cb_03(); void op_cb_04(); void op_cb_05(); void op_cb_06(); void op_cb_07(); void op_cb_08(); void op_cb_09(); void op_cb_0a(); void op_cb_0b(); void op_cb_0c(); void op_cb_0d(); void op_cb_0e(); void op_cb_0f();
  void op_cb_10(); void op_cb_11(); void op_cb_12(); void op_cb_13(); void op_cb_14(); void op_cb_15(); void op_cb_16(); void op_cb_17(); void op_cb_18(); void op_cb_19(); void op_cb_1a(); void op_cb_1b(); void op_cb_1c(); void op_cb_1d(); void op_cb_1e(); void op_cb_1f();
  void op_cb_20(); void op_cb_21(); void op_cb_22(); void op_cb_23(); void op_cb_24(); void op_cb_25(); void op_cb_26(); void op_cb_27(); void op_cb_28(); void op_cb_29(); void op_cb_2a(); void op_cb_2b(); void op_cb_2c(); void op_cb_2d(); void op_cb_2e(); void op_cb_2f();
  void op_cb_30(); void op_cb_31(); void op_cb_32(); void op_cb_33(); void op_cb_34(); void op_cb_35(); void op_cb_36(); void op_cb_37(); void op_cb_38(); void op_cb_39(); void op_cb_3a(); void op_cb_3b(); void op_cb_3c(); void op_cb_3d(); void op_cb_3e(); void op_cb_3f();
  void op_cb_40(); void op_cb_41(); void op_cb_42(); void op_cb_43(); void op_cb_44(); void op_cb_45(); void op_cb_46(); void op_cb_47(); void op_cb_48(); void op_cb_49(); void op_cb_4a(); void op_cb_4b(); void op_cb_4c(); void op_cb_4d(); void op_cb_4e(); void op_cb_4f();
  void op_cb_50(); void op_cb_51(); void op_cb_52(); void op_cb_53(); void op_cb_54(); void op_cb_55(); void op_cb_56(); void op_cb_57(); void op_cb_58(); void op_cb_59(); void op_cb_5a(); void op_cb_5b(); void op_cb_5c(); void op_cb_5d(); void op_cb_5e(); void op_cb_5f();
  void op_cb_60(); void op_cb_61(); void op_cb_62(); void op_cb_63(); void op_cb_64(); void op_cb_65(); void op_cb_66(); void op_cb_67(); void op_cb_68(); void op_cb_69(); void op_cb_6a(); void op_cb_6b(); void op_cb_6c(); void op_cb_6d(); void op_cb_6e(); void op_cb_6f();
  void op_cb_70(); void op_cb_71(); void op_cb_72(); void op_cb_73(); void op_cb_74(); void op_cb_75(); void op_cb_76(); void op_cb_77(); void op_cb_78(); void op_cb_79(); void op_cb_7a(); void op_cb_7b(); void op_cb_7c(); void op_cb_7d(); void op_cb_7e(); void op_cb_7f();
  void op_cb_80(); void op_cb_81(); void op_cb_82(); void op_cb_83(); void op_cb_84(); void op_cb_85(); void op_cb_86(); void op_cb_87(); void op_cb_88(); void op_cb_89(); void op_cb_8a(); void op_cb_8b(); void op_cb_8c(); void op_cb_8d(); void op_cb_8e(); void op_cb_8f();
  void op_cb_90(); void op_cb_91(); void op_cb_92(); void op_cb_93(); void op_cb_94(); void op_cb_95(); void op_cb_96(); void op_cb_97(); void op_cb_98(); void op_cb_99(); void op_cb_9a(); void op_cb_9b(); void op_cb_9c(); void op_cb_9d(); void op_cb_9e(); void op_cb_9f();
  void op_cb_a0(); void op_cb_a1(); void op_cb_a2(); void op_cb_a3(); void op_cb_a4(); void op_cb_a5(); void op_cb_a6(); void op_cb_a7(); void op_cb_a8(); void op_cb_a9(); void op_cb_aa(); void op_cb_ab(); void op_cb_ac(); void op_cb_ad(); void op_cb_ae(); void op_cb_af();
  void op_cb_b0(); void op_cb_b1(); void op_cb_b2(); void op_cb_b3(); void op_cb_b4(); void op_cb_b5(); void op_cb_b6(); void op_cb_b7(); void op_cb_b8(); void op_cb_b9(); void op_cb_ba(); void op_cb_bb(); void op_cb_bc(); void op_cb_bd(); void op_cb_be(); void op_cb_bf();
  void op_cb_c0(); void op_cb_c1(); void op_cb_c2(); void op_cb_c3(); void op_cb_c4(); void op_cb_c5(); void op_cb_c6(); void op_cb_c7(); void op_cb_c8(); void op_cb_c9(); void op_cb_ca(); void op_cb_cb(); void op_cb_cc(); void op_cb_cd(); void op_cb_ce(); void op_cb_cf();
  void op_cb_d0(); void op_cb_d1(); void op_cb_d2(); void op_cb_d3(); void op_cb_d4(); void op_cb_d5(); void op_cb_d6(); void op_cb_d7(); void op_cb_d8(); void op_cb_d9(); void op_cb_da(); void op_cb_db(); void op_cb_dc(); void op_cb_dd(); void op_cb_de(); void op_cb_df();
  void op_cb_e0(); void op_cb_e1(); void op_cb_e2(); void op_cb_e3(); void op_cb_e4(); void op_cb_e5(); void op_cb_e6(); void op_cb_e7(); void op_cb_e8(); void op_cb_e9(); void op_cb_ea(); void op_cb_eb(); void op_cb_ec(); void op_cb_ed(); void op_cb_ee(); void op_cb_ef();
  void op_cb_f0(); void op_cb_f1(); void op_cb_f2(); void op_cb_f3(); void op_cb_f4(); void op_cb_f5(); void op_cb_f6(); void op_cb_f7(); void op_cb_f8(); void op_cb_f9(); void op_cb_fa(); void op_cb_fb(); void op_cb_fc(); void op_cb_fd(); void op_cb_fe(); void op_cb_ff();

  friend class CpuTest;
  friend class CpuTest_op_00_Test;
  friend class CpuTest_op_01_Test;
  friend class CpuTest_op_02_Test;
  friend class CpuTest_op_03_Test;
  friend class CpuTest_op_04_Test;
  friend class CpuTest_op_05_Test;
  friend class CpuTest_op_06_Test;
  friend class CpuTest_op_07_Test;
  friend class CpuTest_op_08_Test;
  friend class CpuTest_op_09_Test;
  friend class CpuTest_op_0a_Test;
  friend class CpuTest_op_0b_Test;
  friend class CpuTest_op_0c_Test;
  friend class CpuTest_op_0d_Test;
  friend class CpuTest_op_0e_Test;
  friend class CpuTest_op_0f_Test;
  friend class CpuTest_op_10_Test;
  friend class CpuTest_op_11_Test;
  friend class CpuTest_op_12_Test;
  friend class CpuTest_op_13_Test;
  friend class CpuTest_op_14_Test;
  friend class CpuTest_op_15_Test;
  friend class CpuTest_op_16_Test;
  friend class CpuTest_op_17_Test;
  friend class CpuTest_op_18_Test;
  friend class CpuTest_op_19_Test;
  friend class CpuTest_op_1a_Test;
  friend class CpuTest_op_1b_Test;
  friend class CpuTest_op_1c_Test;
  friend class CpuTest_op_1d_Test;
  friend class CpuTest_op_1e_Test;
  friend class CpuTest_op_1f_Test;
  friend class CpuTest_op_20_Test;
  friend class CpuTest_op_21_Test;
  friend class CpuTest_op_22_Test;
  friend class CpuTest_op_23_Test;
  friend class CpuTest_op_24_Test;
  friend class CpuTest_op_25_Test;
  friend class CpuTest_op_26_Test;
  friend class CpuTest_op_27_Test;
  friend class CpuTest_op_28_Test;
  friend class CpuTest_op_29_Test;
  friend class CpuTest_op_2a_Test;
  friend class CpuTest_op_2b_Test;
  friend class CpuTest_op_2c_Test;
  friend class CpuTest_op_2d_Test;
  friend class CpuTest_op_2e_Test;
  friend class CpuTest_op_2f_Test;
  friend class CpuTest_op_30_Test;
  friend class CpuTest_op_31_Test;
  friend class CpuTest_op_32_Test;
  friend class CpuTest_op_33_Test;
  friend class CpuTest_op_34_Test;
  friend class CpuTest_op_35_Test;
  friend class CpuTest_op_36_Test;
  friend class CpuTest_op_37_Test;
  friend class CpuTest_op_38_Test;
  friend class CpuTest_op_39_Test;
  friend class CpuTest_op_3a_Test;
  friend class CpuTest_op_3b_Test;
  friend class CpuTest_op_3c_Test;
  friend class CpuTest_op_3d_Test;
  friend class CpuTest_op_3e_Test;
  friend class CpuTest_op_3f_Test;
  friend class CpuTest_op_40_Test;
  friend class CpuTest_op_41_Test;
  friend class CpuTest_op_42_Test;
  friend class CpuTest_op_43_Test;
  friend class CpuTest_op_44_Test;
  friend class CpuTest_op_45_Test;
  friend class CpuTest_op_46_Test;
  friend class CpuTest_op_47_Test;
  friend class CpuTest_op_48_Test;
  friend class CpuTest_op_49_Test;
  friend class CpuTest_op_4a_Test;
  friend class CpuTest_op_4b_Test;
  friend class CpuTest_op_4c_Test;
  friend class CpuTest_op_4d_Test;
  friend class CpuTest_op_4e_Test;
  friend class CpuTest_op_4f_Test;
  friend class CpuTest_op_50_Test;
  friend class CpuTest_op_51_Test;
  friend class CpuTest_op_52_Test;
  friend class CpuTest_op_53_Test;
  friend class CpuTest_op_54_Test;
  friend class CpuTest_op_55_Test;
  friend class CpuTest_op_56_Test;
  friend class CpuTest_op_57_Test;
  friend class CpuTest_op_58_Test;
  friend class CpuTest_op_59_Test;
  friend class CpuTest_op_5a_Test;
  friend class CpuTest_op_5b_Test;
  friend class CpuTest_op_5c_Test;
  friend class CpuTest_op_5d_Test;
  friend class CpuTest_op_5e_Test;
  friend class CpuTest_op_5f_Test;
  friend class CpuTest_op_60_Test;
  friend class CpuTest_op_61_Test;
  friend class CpuTest_op_62_Test;
  friend class CpuTest_op_63_Test;
  friend class CpuTest_op_64_Test;
  friend class CpuTest_op_65_Test;
  friend class CpuTest_op_66_Test;
  friend class CpuTest_op_67_Test;
  friend class CpuTest_op_68_Test;
  friend class CpuTest_op_69_Test;
  friend class CpuTest_op_6a_Test;
  friend class CpuTest_op_6b_Test;
  friend class CpuTest_op_6c_Test;
  friend class CpuTest_op_6d_Test;
  friend class CpuTest_op_6e_Test;
  friend class CpuTest_op_6f_Test;
  friend class CpuTest_op_70_Test;
  friend class CpuTest_op_71_Test;
  friend class CpuTest_op_72_Test;
  friend class CpuTest_op_73_Test;
  friend class CpuTest_op_74_Test;
  friend class CpuTest_op_75_Test;
  friend class CpuTest_op_76_Test;
  friend class CpuTest_op_77_Test;
  friend class CpuTest_op_78_Test;
  friend class CpuTest_op_79_Test;
  friend class CpuTest_op_7a_Test;
  friend class CpuTest_op_7b_Test;
  friend class CpuTest_op_7c_Test;
  friend class CpuTest_op_7d_Test;
  friend class CpuTest_op_7e_Test;
  friend class CpuTest_op_7f_Test;
  friend class CpuTest_op_80_Test;
  friend class CpuTest_op_81_Test;
  friend class CpuTest_op_82_Test;
  friend class CpuTest_op_83_Test;
  friend class CpuTest_op_84_Test;
  friend class CpuTest_op_85_Test;
  friend class CpuTest_op_86_Test;
  friend class CpuTest_op_87_Test;
  friend class CpuTest_op_88_Test;
  friend class CpuTest_op_89_Test;
  friend class CpuTest_op_8a_Test;
  friend class CpuTest_op_8b_Test;
  friend class CpuTest_op_8c_Test;
  friend class CpuTest_op_8d_Test;
  friend class CpuTest_op_8e_Test;
  friend class CpuTest_op_8f_Test;
  friend class CpuTest_op_90_Test;
  friend class CpuTest_op_91_Test;
  friend class CpuTest_op_92_Test;
  friend class CpuTest_op_93_Test;
  friend class CpuTest_op_94_Test;
  friend class CpuTest_op_95_Test;
  friend class CpuTest_op_96_Test;
  friend class CpuTest_op_97_Test;
  friend class CpuTest_op_98_Test;
  friend class CpuTest_op_99_Test;
  friend class CpuTest_op_9a_Test;
  friend class CpuTest_op_9b_Test;
  friend class CpuTest_op_9c_Test;
  friend class CpuTest_op_9d_Test;
  friend class CpuTest_op_9e_Test;
  friend class CpuTest_op_9f_Test;
  friend class CpuTest_op_a0_Test;
  friend class CpuTest_op_a1_Test;
  friend class CpuTest_op_a2_Test;
  friend class CpuTest_op_a3_Test;
  friend class CpuTest_op_a4_Test;
  friend class CpuTest_op_a5_Test;
  friend class CpuTest_op_a6_Test;
  friend class CpuTest_op_a7_Test;
  friend class CpuTest_op_a8_Test;
  friend class CpuTest_op_a9_Test;
  friend class CpuTest_op_aa_Test;
  friend class CpuTest_op_ab_Test;
  friend class CpuTest_op_ac_Test;
  friend class CpuTest_op_ad_Test;
  friend class CpuTest_op_ae_Test;
  friend class CpuTest_op_af_Test;
  friend class CpuTest_op_b0_Test;
  friend class CpuTest_op_b1_Test;
  friend class CpuTest_op_b2_Test;
  friend class CpuTest_op_b3_Test;
  friend class CpuTest_op_b4_Test;
  friend class CpuTest_op_b5_Test;
  friend class CpuTest_op_b6_Test;
  friend class CpuTest_op_b7_Test;
  friend class CpuTest_op_b8_Test;
  friend class CpuTest_op_b9_Test;
  friend class CpuTest_op_ba_Test;
  friend class CpuTest_op_bb_Test;
  friend class CpuTest_op_bc_Test;
  friend class CpuTest_op_bd_Test;
  friend class CpuTest_op_be_Test;
  friend class CpuTest_op_bf_Test;
  friend class CpuTest_op_c0_Test;
  friend class CpuTest_op_c1_Test;
  friend class CpuTest_op_c2_Test;
  friend class CpuTest_op_c3_Test;
  friend class CpuTest_op_c4_Test;
  friend class CpuTest_op_c5_Test;
  friend class CpuTest_op_c6_Test;
  friend class CpuTest_op_c7_Test;
  friend class CpuTest_op_c8_Test;
  friend class CpuTest_op_c9_Test;
  friend class CpuTest_op_ca_Test;
  friend class CpuTest_op_cb_Test;
  friend class CpuTest_op_cc_Test;
  friend class CpuTest_op_cd_Test;
  friend class CpuTest_op_ce_Test;
  friend class CpuTest_op_cf_Test;
  friend class CpuTest_op_d0_Test;
  friend class CpuTest_op_d1_Test;
  friend class CpuTest_op_d2_Test;
  friend class CpuTest_op_d3_Test;
  friend class CpuTest_op_d4_Test;
  friend class CpuTest_op_d5_Test;
  friend class CpuTest_op_d6_Test;
  friend class CpuTest_op_d7_Test;
  friend class CpuTest_op_d8_Test;
  friend class CpuTest_op_d9_Test;
  friend class CpuTest_op_da_Test;
  friend class CpuTest_op_db_Test;
  friend class CpuTest_op_dc_Test;
  friend class CpuTest_op_dd_Test;
  friend class CpuTest_op_de_Test;
  friend class CpuTest_op_df_Test;
  friend class CpuTest_op_e0_Test;
  friend class CpuTest_op_e1_Test;
  friend class CpuTest_op_e2_Test;
  friend class CpuTest_op_e3_Test;
  friend class CpuTest_op_e4_Test;
  friend class CpuTest_op_e5_Test;
  friend class CpuTest_op_e6_Test;
  friend class CpuTest_op_e7_Test;
  friend class CpuTest_op_e8_Test;
  friend class CpuTest_op_e9_Test;
  friend class CpuTest_op_ea_Test;
  friend class CpuTest_op_eb_Test;
  friend class CpuTest_op_ec_Test;
  friend class CpuTest_op_ed_Test;
  friend class CpuTest_op_ee_Test;
  friend class CpuTest_op_ef_Test;
  friend class CpuTest_op_f0_Test;
  friend class CpuTest_op_f1_Test;
  friend class CpuTest_op_f2_Test;
  friend class CpuTest_op_f3_Test;
  friend class CpuTest_op_f4_Test;
  friend class CpuTest_op_f5_Test;
  friend class CpuTest_op_f6_Test;
  friend class CpuTest_op_f7_Test;
  friend class CpuTest_op_f8_Test;
  friend class CpuTest_op_f9_Test;
  friend class CpuTest_op_fa_Test;
  friend class CpuTest_op_fb_Test;
  friend class CpuTest_op_fc_Test;
  friend class CpuTest_op_fd_Test;
  friend class CpuTest_op_fe_Test;
  friend class CpuTest_op_ff_Test;
  friend class CpuTest_op_cb_00_Test;
  friend class CpuTest_op_cb_01_Test;
  friend class CpuTest_op_cb_02_Test;
  friend class CpuTest_op_cb_03_Test;
  friend class CpuTest_op_cb_04_Test;
  friend class CpuTest_op_cb_05_Test;
  friend class CpuTest_op_cb_06_Test;
  friend class CpuTest_op_cb_07_Test;
  friend class CpuTest_op_cb_08_Test;
  friend class CpuTest_op_cb_09_Test;
  friend class CpuTest_op_cb_0a_Test;
  friend class CpuTest_op_cb_0b_Test;
  friend class CpuTest_op_cb_0c_Test;
  friend class CpuTest_op_cb_0d_Test;
  friend class CpuTest_op_cb_0e_Test;
  friend class CpuTest_op_cb_0f_Test;
  friend class CpuTest_op_cb_10_Test;
  friend class CpuTest_op_cb_11_Test;
  friend class CpuTest_op_cb_12_Test;
  friend class CpuTest_op_cb_13_Test;
  friend class CpuTest_op_cb_14_Test;
  friend class CpuTest_op_cb_15_Test;
  friend class CpuTest_op_cb_16_Test;
  friend class CpuTest_op_cb_17_Test;
  friend class CpuTest_op_cb_18_Test;
  friend class CpuTest_op_cb_19_Test;
  friend class CpuTest_op_cb_1a_Test;
  friend class CpuTest_op_cb_1b_Test;
  friend class CpuTest_op_cb_1c_Test;
  friend class CpuTest_op_cb_1d_Test;
  friend class CpuTest_op_cb_1e_Test;
  friend class CpuTest_op_cb_1f_Test;
  friend class CpuTest_op_cb_20_Test;
  friend class CpuTest_op_cb_21_Test;
  friend class CpuTest_op_cb_22_Test;
  friend class CpuTest_op_cb_23_Test;
  friend class CpuTest_op_cb_24_Test;
  friend class CpuTest_op_cb_25_Test;
  friend class CpuTest_op_cb_26_Test;
  friend class CpuTest_op_cb_27_Test;
  friend class CpuTest_op_cb_28_Test;
  friend class CpuTest_op_cb_29_Test;
  friend class CpuTest_op_cb_2a_Test;
  friend class CpuTest_op_cb_2b_Test;
  friend class CpuTest_op_cb_2c_Test;
  friend class CpuTest_op_cb_2d_Test;
  friend class CpuTest_op_cb_2e_Test;
  friend class CpuTest_op_cb_2f_Test;
  friend class CpuTest_op_cb_30_Test;
  friend class CpuTest_op_cb_31_Test;
  friend class CpuTest_op_cb_32_Test;
  friend class CpuTest_op_cb_33_Test;
  friend class CpuTest_op_cb_34_Test;
  friend class CpuTest_op_cb_35_Test;
  friend class CpuTest_op_cb_36_Test;
  friend class CpuTest_op_cb_37_Test;
  friend class CpuTest_op_cb_38_Test;
  friend class CpuTest_op_cb_39_Test;
  friend class CpuTest_op_cb_3a_Test;
  friend class CpuTest_op_cb_3b_Test;
  friend class CpuTest_op_cb_3c_Test;
  friend class CpuTest_op_cb_3d_Test;
  friend class CpuTest_op_cb_3e_Test;
  friend class CpuTest_op_cb_3f_Test;
  friend class CpuTest_op_cb_40_Test;
  friend class CpuTest_op_cb_41_Test;
  friend class CpuTest_op_cb_42_Test;
  friend class CpuTest_op_cb_43_Test;
  friend class CpuTest_op_cb_44_Test;
  friend class CpuTest_op_cb_45_Test;
  friend class CpuTest_op_cb_46_Test;
  friend class CpuTest_op_cb_47_Test;
  friend class CpuTest_op_cb_48_Test;
  friend class CpuTest_op_cb_49_Test;
  friend class CpuTest_op_cb_4a_Test;
  friend class CpuTest_op_cb_4b_Test;
  friend class CpuTest_op_cb_4c_Test;
  friend class CpuTest_op_cb_4d_Test;
  friend class CpuTest_op_cb_4e_Test;
  friend class CpuTest_op_cb_4f_Test;
  friend class CpuTest_op_cb_50_Test;
  friend class CpuTest_op_cb_51_Test;
  friend class CpuTest_op_cb_52_Test;
  friend class CpuTest_op_cb_53_Test;
  friend class CpuTest_op_cb_54_Test;
  friend class CpuTest_op_cb_55_Test;
  friend class CpuTest_op_cb_56_Test;
  friend class CpuTest_op_cb_57_Test;
  friend class CpuTest_op_cb_58_Test;
  friend class CpuTest_op_cb_59_Test;
  friend class CpuTest_op_cb_5a_Test;
  friend class CpuTest_op_cb_5b_Test;
  friend class CpuTest_op_cb_5c_Test;
  friend class CpuTest_op_cb_5d_Test;
  friend class CpuTest_op_cb_5e_Test;
  friend class CpuTest_op_cb_5f_Test;
  friend class CpuTest_op_cb_60_Test;
  friend class CpuTest_op_cb_61_Test;
  friend class CpuTest_op_cb_62_Test;
  friend class CpuTest_op_cb_63_Test;
  friend class CpuTest_op_cb_64_Test;
  friend class CpuTest_op_cb_65_Test;
  friend class CpuTest_op_cb_66_Test;
  friend class CpuTest_op_cb_67_Test;
  friend class CpuTest_op_cb_68_Test;
  friend class CpuTest_op_cb_69_Test;
  friend class CpuTest_op_cb_6a_Test;
  friend class CpuTest_op_cb_6b_Test;
  friend class CpuTest_op_cb_6c_Test;
  friend class CpuTest_op_cb_6d_Test;
  friend class CpuTest_op_cb_6e_Test;
  friend class CpuTest_op_cb_6f_Test;
  friend class CpuTest_op_cb_70_Test;
  friend class CpuTest_op_cb_71_Test;
  friend class CpuTest_op_cb_72_Test;
  friend class CpuTest_op_cb_73_Test;
  friend class CpuTest_op_cb_74_Test;
  friend class CpuTest_op_cb_75_Test;
  friend class CpuTest_op_cb_76_Test;
  friend class CpuTest_op_cb_77_Test;
  friend class CpuTest_op_cb_78_Test;
  friend class CpuTest_op_cb_79_Test;
  friend class CpuTest_op_cb_7a_Test;
  friend class CpuTest_op_cb_7b_Test;
  friend class CpuTest_op_cb_7c_Test;
  friend class CpuTest_op_cb_7d_Test;
  friend class CpuTest_op_cb_7e_Test;
  friend class CpuTest_op_cb_7f_Test;
  friend class CpuTest_op_cb_80_Test;
  friend class CpuTest_op_cb_81_Test;
  friend class CpuTest_op_cb_82_Test;
  friend class CpuTest_op_cb_83_Test;
  friend class CpuTest_op_cb_84_Test;
  friend class CpuTest_op_cb_85_Test;
  friend class CpuTest_op_cb_86_Test;
  friend class CpuTest_op_cb_87_Test;
  friend class CpuTest_op_cb_88_Test;
  friend class CpuTest_op_cb_89_Test;
  friend class CpuTest_op_cb_8a_Test;
  friend class CpuTest_op_cb_8b_Test;
  friend class CpuTest_op_cb_8c_Test;
  friend class CpuTest_op_cb_8d_Test;
  friend class CpuTest_op_cb_8e_Test;
  friend class CpuTest_op_cb_8f_Test;
  friend class CpuTest_op_cb_90_Test;
  friend class CpuTest_op_cb_91_Test;
  friend class CpuTest_op_cb_92_Test;
  friend class CpuTest_op_cb_93_Test;
  friend class CpuTest_op_cb_94_Test;
  friend class CpuTest_op_cb_95_Test;
  friend class CpuTest_op_cb_96_Test;
  friend class CpuTest_op_cb_97_Test;
  friend class CpuTest_op_cb_98_Test;
  friend class CpuTest_op_cb_99_Test;
  friend class CpuTest_op_cb_9a_Test;
  friend class CpuTest_op_cb_9b_Test;
  friend class CpuTest_op_cb_9c_Test;
  friend class CpuTest_op_cb_9d_Test;
  friend class CpuTest_op_cb_9e_Test;
  friend class CpuTest_op_cb_9f_Test;
  friend class CpuTest_op_cb_a0_Test;
  friend class CpuTest_op_cb_a1_Test;
  friend class CpuTest_op_cb_a2_Test;
  friend class CpuTest_op_cb_a3_Test;
  friend class CpuTest_op_cb_a4_Test;
  friend class CpuTest_op_cb_a5_Test;
  friend class CpuTest_op_cb_a6_Test;
  friend class CpuTest_op_cb_a7_Test;
  friend class CpuTest_op_cb_a8_Test;
  friend class CpuTest_op_cb_a9_Test;
  friend class CpuTest_op_cb_aa_Test;
  friend class CpuTest_op_cb_ab_Test;
  friend class CpuTest_op_cb_ac_Test;
  friend class CpuTest_op_cb_ad_Test;
  friend class CpuTest_op_cb_ae_Test;
  friend class CpuTest_op_cb_af_Test;
  friend class CpuTest_op_cb_b0_Test;
  friend class CpuTest_op_cb_b1_Test;
  friend class CpuTest_op_cb_b2_Test;
  friend class CpuTest_op_cb_b3_Test;
  friend class CpuTest_op_cb_b4_Test;
  friend class CpuTest_op_cb_b5_Test;
  friend class CpuTest_op_cb_b6_Test;
  friend class CpuTest_op_cb_b7_Test;
  friend class CpuTest_op_cb_b8_Test;
  friend class CpuTest_op_cb_b9_Test;
  friend class CpuTest_op_cb_ba_Test;
  friend class CpuTest_op_cb_bb_Test;
  friend class CpuTest_op_cb_bc_Test;
  friend class CpuTest_op_cb_bd_Test;
  friend class CpuTest_op_cb_be_Test;
  friend class CpuTest_op_cb_bf_Test;
  friend class CpuTest_op_cb_c0_Test;
  friend class CpuTest_op_cb_c1_Test;
  friend class CpuTest_op_cb_c2_Test;
  friend class CpuTest_op_cb_c3_Test;
  friend class CpuTest_op_cb_c4_Test;
  friend class CpuTest_op_cb_c5_Test;
  friend class CpuTest_op_cb_c6_Test;
  friend class CpuTest_op_cb_c7_Test;
  friend class CpuTest_op_cb_c8_Test;
  friend class CpuTest_op_cb_c9_Test;
  friend class CpuTest_op_cb_ca_Test;
  friend class CpuTest_op_cb_cb_Test;
  friend class CpuTest_op_cb_cc_Test;
  friend class CpuTest_op_cb_cd_Test;
  friend class CpuTest_op_cb_ce_Test;
  friend class CpuTest_op_cb_cf_Test;
  friend class CpuTest_op_cb_d0_Test;
  friend class CpuTest_op_cb_d1_Test;
  friend class CpuTest_op_cb_d2_Test;
  friend class CpuTest_op_cb_d3_Test;
  friend class CpuTest_op_cb_d4_Test;
  friend class CpuTest_op_cb_d5_Test;
  friend class CpuTest_op_cb_d6_Test;
  friend class CpuTest_op_cb_d7_Test;
  friend class CpuTest_op_cb_d8_Test;
  friend class CpuTest_op_cb_d9_Test;
  friend class CpuTest_op_cb_da_Test;
  friend class CpuTest_op_cb_db_Test;
  friend class CpuTest_op_cb_dc_Test;
  friend class CpuTest_op_cb_dd_Test;
  friend class CpuTest_op_cb_de_Test;
  friend class CpuTest_op_cb_df_Test;
  friend class CpuTest_op_cb_e0_Test;
  friend class CpuTest_op_cb_e1_Test;
  friend class CpuTest_op_cb_e2_Test;
  friend class CpuTest_op_cb_e3_Test;
  friend class CpuTest_op_cb_e4_Test;
  friend class CpuTest_op_cb_e5_Test;
  friend class CpuTest_op_cb_e6_Test;
  friend class CpuTest_op_cb_e7_Test;
  friend class CpuTest_op_cb_e8_Test;
  friend class CpuTest_op_cb_e9_Test;
  friend class CpuTest_op_cb_ea_Test;
  friend class CpuTest_op_cb_eb_Test;
  friend class CpuTest_op_cb_ec_Test;
  friend class CpuTest_op_cb_ed_Test;
  friend class CpuTest_op_cb_ee_Test;
  friend class CpuTest_op_cb_ef_Test;
  friend class CpuTest_op_cb_f0_Test;
  friend class CpuTest_op_cb_f1_Test;
  friend class CpuTest_op_cb_f2_Test;
  friend class CpuTest_op_cb_f3_Test;
  friend class CpuTest_op_cb_f4_Test;
  friend class CpuTest_op_cb_f5_Test;
  friend class CpuTest_op_cb_f6_Test;
  friend class CpuTest_op_cb_f7_Test;
  friend class CpuTest_op_cb_f8_Test;
  friend class CpuTest_op_cb_f9_Test;
  friend class CpuTest_op_cb_fa_Test;
  friend class CpuTest_op_cb_fb_Test;
  friend class CpuTest_op_cb_fc_Test;
  friend class CpuTest_op_cb_fd_Test;
  friend class CpuTest_op_cb_fe_Test;
  friend class CpuTest_op_cb_ff_Test;
  /* clang-format on */
};

} // namespace gbc
#endif
