#include "cpu.hh"
#include "log.hh"
#include "register.hh"

namespace gbc {

/* clang-format off */
void Cpu::op_00() { nop(); }
void Cpu::op_01() { ld(bc, d16()); }
void Cpu::op_02() { ld(_(bc), a); }
void Cpu::op_03() { inc(bc); }
void Cpu::op_04() { inc(b); }
void Cpu::op_05() { dec(b); }
void Cpu::op_06() { ld(b); }
void Cpu::op_07() { rlc(a); f.clear_zero_flag(); }
void Cpu::op_08() { ld(a16(), sp); }
void Cpu::op_09() { add(hl, bc); }
void Cpu::op_0a() { ld(a, _(bc)); }
void Cpu::op_0b() { dec(bc); }
void Cpu::op_0c() { inc(c); }
void Cpu::op_0d() { dec(c); }
void Cpu::op_0e() { ld(c); }
void Cpu::op_0f() { rrc(a); f.clear_zero_flag(); }

void Cpu::op_10() { stop(); }
void Cpu::op_11() { ld(de, d16()); }
void Cpu::op_12() { ld(_(de), a); }
void Cpu::op_13() { inc(de); }
void Cpu::op_14() { inc(d); }
void Cpu::op_15() { dec(d); }
void Cpu::op_16() { ld(d); }
void Cpu::op_17() { rl(a); f.clear_zero_flag(); }
void Cpu::op_18() { jr(); }
void Cpu::op_19() { add(hl, de); }
void Cpu::op_1a() { ld(a, _(de)); }
void Cpu::op_1b() { dec(de); }
void Cpu::op_1c() { inc(e); }
void Cpu::op_1d() { dec(e); }
void Cpu::op_1e() { ld(e); }
void Cpu::op_1f() { rr(a); f.clear_zero_flag(); }

void Cpu::op_20() { jr_if(!f.zero_flag()); }
void Cpu::op_21() { ld(hl, d16()); }
void Cpu::op_22() { ldi(_(hl), a); }
void Cpu::op_23() { inc(hl); }
void Cpu::op_24() { inc(h); }
void Cpu::op_25() { dec(h); }
void Cpu::op_26() { ld(h); }
void Cpu::op_27() { daa(); }
void Cpu::op_28() { jr_if(f.zero_flag()); }
void Cpu::op_29() { add(hl, hl); }
void Cpu::op_2a() { ldi(a, _(hl)); }
void Cpu::op_2b() { dec(hl); }
void Cpu::op_2c() { inc(l); }
void Cpu::op_2d() { dec(l); }
void Cpu::op_2e() { ld(l); }
void Cpu::op_2f() { cpl(); }  // CPL

void Cpu::op_30() { jr_if(!f.carry_flag()); }
void Cpu::op_31() { ld(sp, d16()); }
void Cpu::op_32() { ldd(_(hl), a); }
void Cpu::op_33() { inc(sp); }
void Cpu::op_34() { inc(_(hl)); }
void Cpu::op_35() { dec(_(hl)); }
void Cpu::op_36() { ld(_(hl)); }
void Cpu::op_37() { log_error("UNIMPLEMENTED OP CODE 0x37"); }  // SCF
void Cpu::op_38() { jr_if(f.carry_flag()); }
void Cpu::op_39() { add(hl, sp); }
void Cpu::op_3a() { ldd(a, _(hl)); }
void Cpu::op_3b() { dec(sp); }
void Cpu::op_3c() { inc(a); }
void Cpu::op_3d() { dec(a); }
void Cpu::op_3e() { ld(a); }
void Cpu::op_3f() { log_error("UNIMPLEMENTED OP CODE 0x3f"); }  // CCF

void Cpu::op_40() { ld(b, b); }
void Cpu::op_41() { ld(b, c); }
void Cpu::op_42() { ld(b, d); }
void Cpu::op_43() { ld(b, e); }
void Cpu::op_44() { ld(b, h); }
void Cpu::op_45() { ld(b, l); }
void Cpu::op_46() { ld(b, _(hl)); }
void Cpu::op_47() { ld(b, a); }
void Cpu::op_48() { ld(c, b); }
void Cpu::op_49() { ld(c, c); }
void Cpu::op_4a() { ld(c, d); }
void Cpu::op_4b() { ld(c, e); }
void Cpu::op_4c() { ld(c, h); }
void Cpu::op_4d() { ld(c, l); }
void Cpu::op_4e() { ld(c, _(hl)); }
void Cpu::op_4f() { ld(c, a); }

void Cpu::op_50() { ld(d, b); }
void Cpu::op_51() { ld(d, c); }
void Cpu::op_52() { ld(d, d); }
void Cpu::op_53() { ld(d, e); }
void Cpu::op_54() { ld(d, h); }
void Cpu::op_55() { ld(d, l); }
void Cpu::op_56() { ld(d, _(hl)); }
void Cpu::op_57() { ld(d, a); }
void Cpu::op_58() { ld(e, b); }
void Cpu::op_59() { ld(e, c); }
void Cpu::op_5a() { ld(e, d); }
void Cpu::op_5b() { ld(e, e); }
void Cpu::op_5c() { ld(e, h); }
void Cpu::op_5d() { ld(e, l); }
void Cpu::op_5e() { ld(e, _(hl)); }
void Cpu::op_5f() { ld(e, a); }

void Cpu::op_60() { ld(h, b); }
void Cpu::op_61() { ld(h, c); }
void Cpu::op_62() { ld(h, d); }
void Cpu::op_63() { ld(h, e); }
void Cpu::op_64() { ld(h, h); }
void Cpu::op_65() { ld(h, l); }
void Cpu::op_66() { ld(h, _(hl)); }
void Cpu::op_67() { ld(h, a); }
void Cpu::op_68() { ld(l, b); }
void Cpu::op_69() { ld(l, c); }
void Cpu::op_6a() { ld(l, d); }
void Cpu::op_6b() { ld(l, e); }
void Cpu::op_6c() { ld(l, h); }
void Cpu::op_6d() { ld(l, l); }
void Cpu::op_6e() { ld(l, _(hl)); }
void Cpu::op_6f() { ld(l, a); }

void Cpu::op_70() { ld(_(hl), b); }
void Cpu::op_71() { ld(_(hl), c); }
void Cpu::op_72() { ld(_(hl), d); }
void Cpu::op_73() { ld(_(hl), e); }
void Cpu::op_74() { ld(_(hl), h); }
void Cpu::op_75() { ld(_(hl), l); }
void Cpu::op_76() { halt(); }
void Cpu::op_77() { ld(_(hl), a); }
void Cpu::op_78() { ld(a, b); }
void Cpu::op_79() { ld(a, c); }
void Cpu::op_7a() { ld(a, d); }
void Cpu::op_7b() { ld(a, e); }
void Cpu::op_7c() { ld(a, h); }
void Cpu::op_7d() { ld(a, l); }
void Cpu::op_7e() { ld(a, _(hl)); }
void Cpu::op_7f() { ld(a, a); }

void Cpu::op_80() { add(a, b); }
void Cpu::op_81() { add(a, c); }
void Cpu::op_82() { add(a, d); }
void Cpu::op_83() { add(a, e); }
void Cpu::op_84() { add(a, h); }
void Cpu::op_85() { add(a, l); }
void Cpu::op_86() { add(a, _(hl)); }
void Cpu::op_87() { add(a, a); }
void Cpu::op_88() { adc(a, b); }
void Cpu::op_89() { adc(a, c); }
void Cpu::op_8a() { adc(a, d); }
void Cpu::op_8b() { adc(a, e); }
void Cpu::op_8c() { adc(a, h); }
void Cpu::op_8d() { adc(a, l); }
void Cpu::op_8e() { adc(a, _(hl)); }
void Cpu::op_8f() { adc(a, a); }

void Cpu::op_90() { sub(a, b); }
void Cpu::op_91() { sub(a, c); }
void Cpu::op_92() { sub(a, d); }
void Cpu::op_93() { sub(a, e); }
void Cpu::op_94() { sub(a, h); }
void Cpu::op_95() { sub(a, l); }
void Cpu::op_96() { sub(a, _(hl)); }
void Cpu::op_97() { sub(a, a); }
void Cpu::op_98() { sbc(a, b); }
void Cpu::op_99() { sbc(a, c); }
void Cpu::op_9a() { sbc(a, d); }
void Cpu::op_9b() { sbc(a, e); }
void Cpu::op_9c() { sbc(a, h); }
void Cpu::op_9d() { sbc(a, l); }
void Cpu::op_9e() { sbc(a, _(hl)); }
void Cpu::op_9f() { sbc(a, a); }

void Cpu::op_a0() { a_and(b); }
void Cpu::op_a1() { a_and(c); }
void Cpu::op_a2() { a_and(d); }
void Cpu::op_a3() { a_and(e); }
void Cpu::op_a4() { a_and(h); }
void Cpu::op_a5() { a_and(l); }
void Cpu::op_a6() { a_and(_(hl)); }
void Cpu::op_a7() { a_and(a); }
void Cpu::op_a8() { a_xor(b); }
void Cpu::op_a9() { a_xor(c); }
void Cpu::op_aa() { a_xor(d); }
void Cpu::op_ab() { a_xor(e); }
void Cpu::op_ac() { a_xor(h); }
void Cpu::op_ad() { a_xor(l); }
void Cpu::op_ae() { a_xor(_(hl)); }
void Cpu::op_af() { a_xor(a); }

void Cpu::op_b0() { a_or(b); }
void Cpu::op_b1() { a_or(c); }
void Cpu::op_b2() { a_or(d); }
void Cpu::op_b3() { a_or(e); }
void Cpu::op_b4() { a_or(h); }
void Cpu::op_b5() { a_or(l); }
void Cpu::op_b6() { a_or(_(hl)); }
void Cpu::op_b7() { a_or(a); }
void Cpu::op_b8() { cp(b); }
void Cpu::op_b9() { cp(c); }
void Cpu::op_ba() { cp(d); }
void Cpu::op_bb() { cp(e); }
void Cpu::op_bc() { cp(h); }
void Cpu::op_bd() { cp(l); }
void Cpu::op_be() { cp(_(hl)); }
void Cpu::op_bf() { cp(a); }

void Cpu::op_c0() { ret_if(!f.zero_flag()); }
void Cpu::op_c1() { pop(bc); }
void Cpu::op_c2() { jp_if(!f.zero_flag()); }
void Cpu::op_c3() { jp(); }
void Cpu::op_c4() { call_if(!f.zero_flag()); }
void Cpu::op_c5() { push(bc); }
void Cpu::op_c6() { add(a); }
void Cpu::op_c7() { log_error("UNIMPLEMENTED OP CODE 0xc7"); }
void Cpu::op_c8() { ret_if(f.zero_flag()); }
void Cpu::op_c9() { ret(); }
void Cpu::op_ca() { jp_if(f.zero_flag()); }
void Cpu::op_cb() { log_error("UNIMPLEMENTED OP CODE 0xcb"); }
void Cpu::op_cc() { call_if(f.zero_flag()); }
void Cpu::op_cd() { call(); }
void Cpu::op_ce() { adc(a); }
void Cpu::op_cf() { log_error("UNIMPLEMENTED OP CODE 0xcf"); }

void Cpu::op_d0() { ret_if(!f.carry_flag()); }
void Cpu::op_d1() { pop(de); }
void Cpu::op_d2() { jp_if(!f.carry_flag()); }
void Cpu::op_d3() { log_error("[op] illegal opcode 0xd3"); }
void Cpu::op_d4() { call_if(!f.carry_flag()); }
void Cpu::op_d5() { push(de); }
void Cpu::op_d6() { sub(a); }
void Cpu::op_d7() { log_error("UNIMPLEMENTED OP CODE 0xd7"); }
void Cpu::op_d8() { ret_if(f.carry_flag()); }
void Cpu::op_d9() { reti(); }
void Cpu::op_da() { jp_if(f.carry_flag()); }
void Cpu::op_db() { log_error("[op] illegal opcode 0xdb"); }
void Cpu::op_dc() { call_if(f.carry_flag()); }
void Cpu::op_dd() { log_error("[op] illegal opcode 0xdd"); }
void Cpu::op_de() { sbc(a); }
void Cpu::op_df() { log_error("UNIMPLEMENTED OP CODE 0xdf"); }

void Cpu::op_e0() { ldh(next_byte(), a); }
void Cpu::op_e1() { pop(hl); }
void Cpu::op_e2() { ldh(_(c), a); }
void Cpu::op_e3() { log_error("[op] illegal opcode 0xe3"); }
void Cpu::op_e4() { log_error("[op] illegal opcode 0xe4"); }
void Cpu::op_e5() { push(hl); }
void Cpu::op_e6() { a_and(); }
void Cpu::op_e7() { log_error("UNIMPLEMENTED OP CODE 0xe7"); }
void Cpu::op_e8() { log_error("UNIMPLEMENTED OP CODE 0xe8"); }
void Cpu::op_e9() { jp(_(hl)); }
void Cpu::op_ea() { ld(next_word(), a); }
void Cpu::op_eb() { log_error("[op] illegal opcode 0xeb"); }
void Cpu::op_ec() { log_error("[op] illegal opcode 0xec"); }
void Cpu::op_ed() { log_error("[op] illegal opcode 0xed"); }
void Cpu::op_ee() { a_xor(); }
void Cpu::op_ef() { rst(rst::_28); }

void Cpu::op_f0() { ldh(a, next_byte()); }
void Cpu::op_f1() { pop(af); }
void Cpu::op_f2() { log_error("[op] illegal opcode 0xf2"); }
void Cpu::op_f3() { di(); }
void Cpu::op_f4() { log_error("[op] illegal opcode 0xf4"); }
void Cpu::op_f5() { push(af); }
void Cpu::op_f6() { a_or(); }
void Cpu::op_f7() { log_error("UNIMPLEMENTED OP CODE 0xf7"); }
void Cpu::op_f8() { ldhlsp(); }
void Cpu::op_f9() { ld(sp, hl); }
void Cpu::op_fa() { ld(a, next_word()); }
void Cpu::op_fb() { ei(); }
void Cpu::op_fc() { log_error("[op] illegal opcode 0xfc"); }
void Cpu::op_fd() { log_error("[op] illegal opcode 0xfd"); }
void Cpu::op_fe() { cp(); }
void Cpu::op_ff() { log_error("UNIMPLEMENTED OP CODE 0xff"); }

void Cpu::op_cb_00() { rlc(b); }
void Cpu::op_cb_01() { rlc(c); }
void Cpu::op_cb_02() { rlc(d); }
void Cpu::op_cb_03() { rlc(e); }
void Cpu::op_cb_04() { rlc(h); }
void Cpu::op_cb_05() { rlc(l); }
void Cpu::op_cb_06() { rlc(_(hl)); }
void Cpu::op_cb_07() { rlc(a); }
void Cpu::op_cb_08() { rrc(b); }
void Cpu::op_cb_09() { rrc(c); }
void Cpu::op_cb_0a() { rrc(d); }
void Cpu::op_cb_0b() { rrc(e); }
void Cpu::op_cb_0c() { rrc(h); }
void Cpu::op_cb_0d() { rrc(l); }
void Cpu::op_cb_0e() { rrc(_(hl)); }
void Cpu::op_cb_0f() { rrc(a); }

void Cpu::op_cb_10() { rl(b); }
void Cpu::op_cb_11() { rl(c); }
void Cpu::op_cb_12() { rl(d); }
void Cpu::op_cb_13() { rl(e); }
void Cpu::op_cb_14() { rl(h); }
void Cpu::op_cb_15() { rl(l); }
void Cpu::op_cb_16() { rl(_(hl)); }
void Cpu::op_cb_17() { rl(a); }
void Cpu::op_cb_18() { rr(b); }
void Cpu::op_cb_19() { rr(c); }
void Cpu::op_cb_1a() { rr(d); }
void Cpu::op_cb_1b() { rr(e); }
void Cpu::op_cb_1c() { rr(h); }
void Cpu::op_cb_1d() { rr(l); }
void Cpu::op_cb_1e() { rr(_(hl)); }
void Cpu::op_cb_1f() { rr(a); }

void Cpu::op_cb_20() { sla(b); }
void Cpu::op_cb_21() { sla(c); }
void Cpu::op_cb_22() { sla(d); }
void Cpu::op_cb_23() { sla(e); }
void Cpu::op_cb_24() { sla(h); }
void Cpu::op_cb_25() { sla(l); }
void Cpu::op_cb_26() { sla(_(hl)); }
void Cpu::op_cb_27() { sla(a); }
void Cpu::op_cb_28() { sra(b); }
void Cpu::op_cb_29() { sra(c); }
void Cpu::op_cb_2a() { sra(d); }
void Cpu::op_cb_2b() { sra(e); }
void Cpu::op_cb_2c() { sra(h); }
void Cpu::op_cb_2d() { sra(l); }
void Cpu::op_cb_2e() { sra(_(hl)); }
void Cpu::op_cb_2f() { sra(a); }

void Cpu::op_cb_30() { swap(b); }
void Cpu::op_cb_31() { swap(c); }
void Cpu::op_cb_32() { swap(d); }
void Cpu::op_cb_33() { swap(e); }
void Cpu::op_cb_34() { swap(h); }
void Cpu::op_cb_35() { swap(l); }
void Cpu::op_cb_36() { swap(_(hl)); }
void Cpu::op_cb_37() { swap(a); }
void Cpu::op_cb_38() { srl(b); }
void Cpu::op_cb_39() { srl(c); }
void Cpu::op_cb_3a() { srl(d); }
void Cpu::op_cb_3b() { srl(e); }
void Cpu::op_cb_3c() { srl(h); }
void Cpu::op_cb_3d() { srl(l); }
void Cpu::op_cb_3e() { srl(_(hl)); }
void Cpu::op_cb_3f() { srl(a); }

void Cpu::op_cb_40() { bit(0, b); }
void Cpu::op_cb_41() { bit(0, c); }
void Cpu::op_cb_42() { bit(0, d); }
void Cpu::op_cb_43() { bit(0, e); }
void Cpu::op_cb_44() { bit(0, h); }
void Cpu::op_cb_45() { bit(0, l); }
void Cpu::op_cb_46() { bit(0, _(hl)); }
void Cpu::op_cb_47() { bit(0, a); }
void Cpu::op_cb_48() { bit(1, b); }
void Cpu::op_cb_49() { bit(1, c); }
void Cpu::op_cb_4a() { bit(1, d); }
void Cpu::op_cb_4b() { bit(1, e); }
void Cpu::op_cb_4c() { bit(1, h); }
void Cpu::op_cb_4d() { bit(1, l); }
void Cpu::op_cb_4e() { bit(1, _(hl)); }
void Cpu::op_cb_4f() { bit(1, a); }

void Cpu::op_cb_50() { bit(2, b); }
void Cpu::op_cb_51() { bit(2, c); }
void Cpu::op_cb_52() { bit(2, d); }
void Cpu::op_cb_53() { bit(2, e); }
void Cpu::op_cb_54() { bit(2, h); }
void Cpu::op_cb_55() { bit(2, l); }
void Cpu::op_cb_56() { bit(2, _(hl)); }
void Cpu::op_cb_57() { bit(2, a); }
void Cpu::op_cb_58() { bit(3, b); }
void Cpu::op_cb_59() { bit(3, c); }
void Cpu::op_cb_5a() { bit(3, d); }
void Cpu::op_cb_5b() { bit(3, e); }
void Cpu::op_cb_5c() { bit(3, h); }
void Cpu::op_cb_5d() { bit(3, l); }
void Cpu::op_cb_5e() { bit(3, _(hl)); }
void Cpu::op_cb_5f() { bit(3, a); }

void Cpu::op_cb_60() { bit(4, b); }
void Cpu::op_cb_61() { bit(4, c); }
void Cpu::op_cb_62() { bit(4, d); }
void Cpu::op_cb_63() { bit(4, e); }
void Cpu::op_cb_64() { bit(4, h); }
void Cpu::op_cb_65() { bit(4, l); }
void Cpu::op_cb_66() { bit(4, _(hl)); }
void Cpu::op_cb_67() { bit(4, a); }
void Cpu::op_cb_68() { bit(5, b); }
void Cpu::op_cb_69() { bit(5, c); }
void Cpu::op_cb_6a() { bit(5, d); }
void Cpu::op_cb_6b() { bit(5, e); }
void Cpu::op_cb_6c() { bit(5, h); }
void Cpu::op_cb_6d() { bit(5, l); }
void Cpu::op_cb_6e() { bit(5, _(hl)); }
void Cpu::op_cb_6f() { bit(5, a); }

void Cpu::op_cb_70() { bit(6, b); }
void Cpu::op_cb_71() { bit(6, c); }
void Cpu::op_cb_72() { bit(6, d); }
void Cpu::op_cb_73() { bit(6, e); }
void Cpu::op_cb_74() { bit(6, h); }
void Cpu::op_cb_75() { bit(6, l); }
void Cpu::op_cb_76() { bit(6, _(hl)); }
void Cpu::op_cb_77() { bit(6, a); }
void Cpu::op_cb_78() { bit(7, b); }
void Cpu::op_cb_79() { bit(7, c); }
void Cpu::op_cb_7a() { bit(7, d); }
void Cpu::op_cb_7b() { bit(7, e); }
void Cpu::op_cb_7c() { bit(7, h); }
void Cpu::op_cb_7d() { bit(7, l); }
void Cpu::op_cb_7e() { bit(7, _(hl)); }
void Cpu::op_cb_7f() { bit(7, a); }

void Cpu::op_cb_80() { res(0, b); }
void Cpu::op_cb_81() { res(0, c); }
void Cpu::op_cb_82() { res(0, d); }
void Cpu::op_cb_83() { res(0, e); }
void Cpu::op_cb_84() { res(0, h); }
void Cpu::op_cb_85() { res(0, l); }
void Cpu::op_cb_86() { res(0, _(hl)); }
void Cpu::op_cb_87() { res(0, a); }
void Cpu::op_cb_88() { res(1, b); }
void Cpu::op_cb_89() { res(1, c); }
void Cpu::op_cb_8a() { res(1, d); }
void Cpu::op_cb_8b() { res(1, e); }
void Cpu::op_cb_8c() { res(1, h); }
void Cpu::op_cb_8d() { res(1, l); }
void Cpu::op_cb_8e() { res(1, _(hl)); }
void Cpu::op_cb_8f() { res(1, a); }

void Cpu::op_cb_90() { res(2, b); }
void Cpu::op_cb_91() { res(2, c); }
void Cpu::op_cb_92() { res(2, d); }
void Cpu::op_cb_93() { res(2, e); }
void Cpu::op_cb_94() { res(2, h); }
void Cpu::op_cb_95() { res(2, l); }
void Cpu::op_cb_96() { res(2, _(hl)); }
void Cpu::op_cb_97() { res(2, a); }
void Cpu::op_cb_98() { res(3, b); }
void Cpu::op_cb_99() { res(3, c); }
void Cpu::op_cb_9a() { res(3, d); }
void Cpu::op_cb_9b() { res(3, e); }
void Cpu::op_cb_9c() { res(3, h); }
void Cpu::op_cb_9d() { res(3, l); }
void Cpu::op_cb_9e() { res(3, _(hl)); }
void Cpu::op_cb_9f() { res(3, a); }

void Cpu::op_cb_a0() { res(4, b); }
void Cpu::op_cb_a1() { res(4, c); }
void Cpu::op_cb_a2() { res(4, d); }
void Cpu::op_cb_a3() { res(4, e); }
void Cpu::op_cb_a4() { res(4, h); }
void Cpu::op_cb_a5() { res(4, l); }
void Cpu::op_cb_a6() { res(4, _(hl)); }
void Cpu::op_cb_a7() { res(4, a); }
void Cpu::op_cb_a8() { res(5, b); }
void Cpu::op_cb_a9() { res(5, c); }
void Cpu::op_cb_aa() { res(5, d); }
void Cpu::op_cb_ab() { res(5, e); }
void Cpu::op_cb_ac() { res(5, h); }
void Cpu::op_cb_ad() { res(5, l); }
void Cpu::op_cb_ae() { res(5, _(hl)); }
void Cpu::op_cb_af() { res(5, a); }

void Cpu::op_cb_b0() { res(6, b); }
void Cpu::op_cb_b1() { res(6, c); }
void Cpu::op_cb_b2() { res(6, d); }
void Cpu::op_cb_b3() { res(6, e); }
void Cpu::op_cb_b4() { res(6, h); }
void Cpu::op_cb_b5() { res(6, l); }
void Cpu::op_cb_b6() { res(6, _(hl)); }
void Cpu::op_cb_b7() { res(6, a); }
void Cpu::op_cb_b8() { res(7, b); }
void Cpu::op_cb_b9() { res(7, c); }
void Cpu::op_cb_ba() { res(7, d); }
void Cpu::op_cb_bb() { res(7, e); }
void Cpu::op_cb_bc() { res(7, h); }
void Cpu::op_cb_bd() { res(7, l); }
void Cpu::op_cb_be() { res(7, _(hl)); }
void Cpu::op_cb_bf() { res(7, a); }

void Cpu::op_cb_c0() { set(0, b); }
void Cpu::op_cb_c1() { set(0, c); }
void Cpu::op_cb_c2() { set(0, d); }
void Cpu::op_cb_c3() { set(0, e); }
void Cpu::op_cb_c4() { set(0, h); }
void Cpu::op_cb_c5() { set(0, l); }
void Cpu::op_cb_c6() { set(0, _(hl)); }
void Cpu::op_cb_c7() { set(0, a); }
void Cpu::op_cb_c8() { set(1, b); }
void Cpu::op_cb_c9() { set(1, c); }
void Cpu::op_cb_ca() { set(1, d); }
void Cpu::op_cb_cb() { set(1, e); }
void Cpu::op_cb_cc() { set(1, h); }
void Cpu::op_cb_cd() { set(1, l); }
void Cpu::op_cb_ce() { set(1, _(hl)); }
void Cpu::op_cb_cf() { set(1, a); }

void Cpu::op_cb_d0() { set(2, b); }
void Cpu::op_cb_d1() { set(2, c); }
void Cpu::op_cb_d2() { set(2, d); }
void Cpu::op_cb_d3() { set(2, e); }
void Cpu::op_cb_d4() { set(2, h); }
void Cpu::op_cb_d5() { set(2, l); }
void Cpu::op_cb_d6() { set(2, _(hl)); }
void Cpu::op_cb_d7() { set(2, a); }
void Cpu::op_cb_d8() { set(3, b); }
void Cpu::op_cb_d9() { set(3, c); }
void Cpu::op_cb_da() { set(3, d); }
void Cpu::op_cb_db() { set(3, e); }
void Cpu::op_cb_dc() { set(3, h); }
void Cpu::op_cb_dd() { set(3, l); }
void Cpu::op_cb_de() { set(3, _(hl)); }
void Cpu::op_cb_df() { set(3, a); }

void Cpu::op_cb_e0() { set(4, b); }
void Cpu::op_cb_e1() { set(4, c); }
void Cpu::op_cb_e2() { set(4, d); }
void Cpu::op_cb_e3() { set(4, e); }
void Cpu::op_cb_e4() { set(4, h); }
void Cpu::op_cb_e5() { set(4, l); }
void Cpu::op_cb_e6() { set(4, _(hl)); }
void Cpu::op_cb_e7() { set(4, a); }
void Cpu::op_cb_e8() { set(5, b); }
void Cpu::op_cb_e9() { set(5, c); }
void Cpu::op_cb_ea() { set(5, d); }
void Cpu::op_cb_eb() { set(5, e); }
void Cpu::op_cb_ec() { set(5, h); }
void Cpu::op_cb_ed() { set(5, l); }
void Cpu::op_cb_ee() { set(5, _(hl)); }
void Cpu::op_cb_ef() { set(5, a); }

void Cpu::op_cb_f0() { set(6, b); }
void Cpu::op_cb_f1() { set(6, c); }
void Cpu::op_cb_f2() { set(6, d); }
void Cpu::op_cb_f3() { set(6, e); }
void Cpu::op_cb_f4() { set(6, h); }
void Cpu::op_cb_f5() { set(6, l); }
void Cpu::op_cb_f6() { set(6, _(hl)); }
void Cpu::op_cb_f7() { set(6, a); }
void Cpu::op_cb_f8() { set(7, b); }
void Cpu::op_cb_f9() { set(7, c); }
void Cpu::op_cb_fa() { set(7, d); }
void Cpu::op_cb_fb() { set(7, e); }
void Cpu::op_cb_fc() { set(7, h); }
void Cpu::op_cb_fd() { set(7, l); }
void Cpu::op_cb_fe() { set(7, _(hl)); }
void Cpu::op_cb_ff() { set(7, a); }
/* clang-format on */

} // namespace gbc
