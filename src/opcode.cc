#include "cpu.hh"

namespace gbc {

/* clang-format off */
void Cpu::op_00() { nop(); }
void Cpu::op_01() { ld(bc); }
void Cpu::op_02() { ld(bc.value(), a); }
void Cpu::op_03() { inc(bc); }
void Cpu::op_04() { inc(b); }
void Cpu::op_05() { dec(b); }
void Cpu::op_06() { ld(b); }
void Cpu::op_07() { rlc(a); f.clear_zero_flag(); }
void Cpu::op_08() { ld(step_pc(), sp); }
void Cpu::op_09() { /* TODO */ }  // ADD
void Cpu::op_0a() { ld(a, bc.value()); }
void Cpu::op_0b() { dec(bc); }
void Cpu::op_0c() { inc(c); }
void Cpu::op_0d() { dec(c); }
void Cpu::op_0e() { ld(c); }
void Cpu::op_0f() { rrc(a); f.clear_zero_flag(); }
void Cpu::op_10() { /* TODO */ }  // STOP
void Cpu::op_11() { ld(de); }
void Cpu::op_12() { ld(de.value(), a); }
void Cpu::op_13() { inc(de); }
void Cpu::op_14() { inc(d); }
void Cpu::op_15() { dec(d); }
void Cpu::op_16() { ld(d); }
void Cpu::op_17() { /* TODO */ }  // RL
void Cpu::op_18() { /* TODO */ }  // JR
void Cpu::op_19() { /* TODO */ }  // ADD
void Cpu::op_1a() { ld(a, de.value()); }
void Cpu::op_1b() { dec(de); }
void Cpu::op_1c() { inc(e); }
void Cpu::op_1d() { dec(e); }
void Cpu::op_1e() { ld(e); }
void Cpu::op_1f() { /* TODO */ }  // RR
void Cpu::op_20() { /* TODO */ }  // JR
void Cpu::op_21() { ld(hl); }
void Cpu::op_22() { /* TODO */ }  // LDI
void Cpu::op_23() { inc(hl); }
void Cpu::op_24() { inc(h); }
void Cpu::op_25() { dec(h); }
void Cpu::op_26() { ld(h); }
void Cpu::op_27() { /* TODO */ }  // DAA
void Cpu::op_28() { /* TODO */ }  // JR
void Cpu::op_29() { /* TODO */ }  // ADD
void Cpu::op_2a() { /* TODO */ }  // LDI
void Cpu::op_2b() { dec(hl); }
void Cpu::op_2c() { inc(l); }
void Cpu::op_2d() { dec(l); }
void Cpu::op_2e() { ld(l); }
void Cpu::op_2f() { /* TODO */ }  // CPL
void Cpu::op_30() { /* TODO */ }  // JR
void Cpu::op_31() { ld(sp); }
void Cpu::op_32() { /* TODO */ }  // LDD
void Cpu::op_33() { inc(sp); }
void Cpu::op_34() { inc(hl.value()); }  // INC (HL)
void Cpu::op_35() { dec(hl.value()); }  // DEC (HL)
void Cpu::op_36() { ld(hl.value()); }
void Cpu::op_37() { /* TODO */ }  // SCF
void Cpu::op_38() { /* TODO */ }  // JR
void Cpu::op_39() { /* TODO */ }  // ADD
void Cpu::op_3a() { /* TODO */ }  // LDD
void Cpu::op_3b() { dec(sp); }
void Cpu::op_3c() { inc(a); }
void Cpu::op_3d() { dec(a); }
void Cpu::op_3e() { ld(a); }
void Cpu::op_3f() { /* TODO */ }  // CCF
void Cpu::op_40() { ld(b, b); }
void Cpu::op_41() { ld(b, c); }
void Cpu::op_42() { ld(b, d); }
void Cpu::op_43() { ld(b, e); }
void Cpu::op_44() { ld(b, h); }
void Cpu::op_45() { ld(b, l); }
void Cpu::op_46() { ld(b, hl.value()); }
void Cpu::op_47() { ld(b, a); }
void Cpu::op_48() { ld(c, b); }
void Cpu::op_49() { ld(c, c); }
void Cpu::op_4a() { ld(c, d); }
void Cpu::op_4b() { ld(c, e); }
void Cpu::op_4c() { ld(c, h); }
void Cpu::op_4d() { ld(c, l); }
void Cpu::op_4e() { ld(c, hl.value()); }
void Cpu::op_4f() { ld(c, a); }
void Cpu::op_50() { ld(d, b); }
void Cpu::op_51() { ld(d, c); }
void Cpu::op_52() { ld(d, d); }
void Cpu::op_53() { ld(d, e); }
void Cpu::op_54() { ld(d, h); }
void Cpu::op_55() { ld(d, l); }
void Cpu::op_56() { ld(d, hl.value()); }
void Cpu::op_57() { ld(d, a); }
void Cpu::op_58() { ld(e, b); }
void Cpu::op_59() { ld(e, c); }
void Cpu::op_5a() { ld(e, d); }
void Cpu::op_5b() { ld(e, e); }
void Cpu::op_5c() { ld(e, h); }
void Cpu::op_5d() { ld(e, l); }
void Cpu::op_5e() { ld(e, hl.value()); }
void Cpu::op_5f() { ld(e, a); }
void Cpu::op_60() { ld(h, b); }
void Cpu::op_61() { ld(h, c); }
void Cpu::op_62() { ld(h, d); }
void Cpu::op_63() { ld(h, e); }
void Cpu::op_64() { ld(h, h); }
void Cpu::op_65() { ld(h, l); }
void Cpu::op_66() { ld(h, hl.value()); }
void Cpu::op_67() { ld(h, a); }
void Cpu::op_68() { ld(l, b); }
void Cpu::op_69() { ld(l, c); }
void Cpu::op_6a() { ld(l, d); }
void Cpu::op_6b() { ld(l, e); }
void Cpu::op_6c() { ld(l, h); }
void Cpu::op_6d() { ld(l, l); }
void Cpu::op_6e() { ld(l, hl.value()); }
void Cpu::op_6f() { ld(l, a); }
void Cpu::op_70() { ld(hl.value(), b); }
void Cpu::op_71() { ld(hl.value(), c); }
void Cpu::op_72() { ld(hl.value(), d); }
void Cpu::op_73() { ld(hl.value(), e); }
void Cpu::op_74() { ld(hl.value(), h); }
void Cpu::op_75() { ld(hl.value(), l); }
void Cpu::op_76() { /* TODO */ }  // HALT
void Cpu::op_77() { ld(hl.value(), a); }
void Cpu::op_78() { ld(a, b); }
void Cpu::op_79() { ld(a, c); }
void Cpu::op_7a() { ld(a, d); }
void Cpu::op_7b() { ld(a, e); }
void Cpu::op_7c() { ld(a, h); }
void Cpu::op_7d() { ld(a, l); }
void Cpu::op_7e() { ld(a, hl.value()); }
void Cpu::op_7f() { ld(a, a); }
void Cpu::op_80() { /* TODO */ }
void Cpu::op_81() { /* TODO */ }
void Cpu::op_82() { /* TODO */ }
void Cpu::op_83() { /* TODO */ }
void Cpu::op_84() { /* TODO */ }
void Cpu::op_85() { /* TODO */ }
void Cpu::op_86() { /* TODO */ }
void Cpu::op_87() { /* TODO */ }
void Cpu::op_88() { /* TODO */ }
void Cpu::op_89() { /* TODO */ }
void Cpu::op_8a() { /* TODO */ }
void Cpu::op_8b() { /* TODO */ }
void Cpu::op_8c() { /* TODO */ }
void Cpu::op_8d() { /* TODO */ }
void Cpu::op_8e() { /* TODO */ }
void Cpu::op_8f() { /* TODO */ }
void Cpu::op_90() { /* TODO */ }
void Cpu::op_91() { /* TODO */ }
void Cpu::op_92() { /* TODO */ }
void Cpu::op_93() { /* TODO */ }
void Cpu::op_94() { /* TODO */ }
void Cpu::op_95() { /* TODO */ }
void Cpu::op_96() { /* TODO */ }
void Cpu::op_97() { /* TODO */ }
void Cpu::op_98() { /* TODO */ }
void Cpu::op_99() { /* TODO */ }
void Cpu::op_9a() { /* TODO */ }
void Cpu::op_9b() { /* TODO */ }
void Cpu::op_9c() { /* TODO */ }
void Cpu::op_9d() { /* TODO */ }
void Cpu::op_9e() { /* TODO */ }
void Cpu::op_9f() { /* TODO */ }
void Cpu::op_a0() { /* TODO */ }
void Cpu::op_a1() { /* TODO */ }
void Cpu::op_a2() { /* TODO */ }
void Cpu::op_a3() { /* TODO */ }
void Cpu::op_a4() { /* TODO */ }
void Cpu::op_a5() { /* TODO */ }
void Cpu::op_a6() { /* TODO */ }
void Cpu::op_a7() { /* TODO */ }
void Cpu::op_a8() { /* TODO */ }
void Cpu::op_a9() { /* TODO */ }
void Cpu::op_aa() { /* TODO */ }
void Cpu::op_ab() { /* TODO */ }
void Cpu::op_ac() { /* TODO */ }
void Cpu::op_ad() { /* TODO */ }
void Cpu::op_ae() { /* TODO */ }
void Cpu::op_af() { /* TODO */ }
void Cpu::op_b0() { /* TODO */ }
void Cpu::op_b1() { /* TODO */ }
void Cpu::op_b2() { /* TODO */ }
void Cpu::op_b3() { /* TODO */ }
void Cpu::op_b4() { /* TODO */ }
void Cpu::op_b5() { /* TODO */ }
void Cpu::op_b6() { /* TODO */ }
void Cpu::op_b7() { /* TODO */ }
void Cpu::op_b8() { /* TODO */ }
void Cpu::op_b9() { /* TODO */ }
void Cpu::op_ba() { /* TODO */ }
void Cpu::op_bb() { /* TODO */ }
void Cpu::op_bc() { /* TODO */ }
void Cpu::op_bd() { /* TODO */ }
void Cpu::op_be() { /* TODO */ }
void Cpu::op_bf() { /* TODO */ }
void Cpu::op_c0() { /* TODO */ }
void Cpu::op_c1() { /* TODO */ }
void Cpu::op_c2() { /* TODO */ }
void Cpu::op_c3() { /* TODO */ }
void Cpu::op_c4() { /* TODO */ }
void Cpu::op_c5() { /* TODO */ }
void Cpu::op_c6() { /* TODO */ }
void Cpu::op_c7() { /* TODO */ }
void Cpu::op_c8() { /* TODO */ }
void Cpu::op_c9() { /* TODO */ }
void Cpu::op_ca() { /* TODO */ }
void Cpu::op_cb() { /* TODO */ }
void Cpu::op_cc() { /* TODO */ }
void Cpu::op_cd() { /* TODO */ }
void Cpu::op_ce() { /* TODO */ }
void Cpu::op_cf() { /* TODO */ }
void Cpu::op_d0() { /* TODO */ }
void Cpu::op_d1() { /* TODO */ }
void Cpu::op_d2() { /* TODO */ }
void Cpu::op_d3() { /* TODO */ }
void Cpu::op_d4() { /* TODO */ }
void Cpu::op_d5() { /* TODO */ }
void Cpu::op_d6() { /* TODO */ }
void Cpu::op_d7() { /* TODO */ }
void Cpu::op_d8() { /* TODO */ }
void Cpu::op_d9() { /* TODO */ }
void Cpu::op_da() { /* TODO */ }
void Cpu::op_db() { /* TODO */ }
void Cpu::op_dc() { /* TODO */ }
void Cpu::op_dd() { /* TODO */ }
void Cpu::op_de() { /* TODO */ }
void Cpu::op_df() { /* TODO */ }
void Cpu::op_e0() { /* TODO */ }
void Cpu::op_e1() { /* TODO */ }
void Cpu::op_e2() { /* TODO */ }
void Cpu::op_e3() { /* TODO */ }
void Cpu::op_e4() { /* TODO */ }
void Cpu::op_e5() { /* TODO */ }
void Cpu::op_e6() { /* TODO */ }
void Cpu::op_e7() { /* TODO */ }
void Cpu::op_e8() { /* TODO */ }
void Cpu::op_e9() { /* TODO */ }
void Cpu::op_ea() { /* TODO */ }
void Cpu::op_eb() { /* TODO */ }
void Cpu::op_ec() { /* TODO */ }
void Cpu::op_ed() { /* TODO */ }
void Cpu::op_ee() { /* TODO */ }
void Cpu::op_ef() { /* TODO */ }
void Cpu::op_f0() { /* TODO */ }
void Cpu::op_f1() { /* TODO */ }
void Cpu::op_f2() { /* TODO */ }
void Cpu::op_f3() { /* TODO */ }
void Cpu::op_f4() { /* TODO */ }
void Cpu::op_f5() { /* TODO */ }
void Cpu::op_f6() { /* TODO */ }
void Cpu::op_f7() { /* TODO */ }
void Cpu::op_f8() { /* TODO */ }
void Cpu::op_f9() { /* TODO */ }
void Cpu::op_fa() { /* TODO */ }
void Cpu::op_fb() { /* TODO */ }
void Cpu::op_fc() { /* TODO */ }
void Cpu::op_fd() { /* TODO */ }
void Cpu::op_fe() { /* TODO */ }
void Cpu::op_ff() { /* TODO */ }
void Cpu::op_cb_00() { /* TODO */ }
void Cpu::op_cb_01() { /* TODO */ }
void Cpu::op_cb_02() { /* TODO */ }
void Cpu::op_cb_03() { /* TODO */ }
void Cpu::op_cb_04() { /* TODO */ }
void Cpu::op_cb_05() { /* TODO */ }
void Cpu::op_cb_06() { /* TODO */ }
void Cpu::op_cb_07() { /* TODO */ }
void Cpu::op_cb_08() { /* TODO */ }
void Cpu::op_cb_09() { /* TODO */ }
void Cpu::op_cb_0a() { /* TODO */ }
void Cpu::op_cb_0b() { /* TODO */ }
void Cpu::op_cb_0c() { /* TODO */ }
void Cpu::op_cb_0d() { /* TODO */ }
void Cpu::op_cb_0e() { /* TODO */ }
void Cpu::op_cb_0f() { /* TODO */ }
void Cpu::op_cb_10() { /* TODO */ }
void Cpu::op_cb_11() { /* TODO */ }
void Cpu::op_cb_12() { /* TODO */ }
void Cpu::op_cb_13() { /* TODO */ }
void Cpu::op_cb_14() { /* TODO */ }
void Cpu::op_cb_15() { /* TODO */ }
void Cpu::op_cb_16() { /* TODO */ }
void Cpu::op_cb_17() { /* TODO */ }
void Cpu::op_cb_18() { /* TODO */ }
void Cpu::op_cb_19() { /* TODO */ }
void Cpu::op_cb_1a() { /* TODO */ }
void Cpu::op_cb_1b() { /* TODO */ }
void Cpu::op_cb_1c() { /* TODO */ }
void Cpu::op_cb_1d() { /* TODO */ }
void Cpu::op_cb_1e() { /* TODO */ }
void Cpu::op_cb_1f() { /* TODO */ }
void Cpu::op_cb_20() { /* TODO */ }
void Cpu::op_cb_21() { /* TODO */ }
void Cpu::op_cb_22() { /* TODO */ }
void Cpu::op_cb_23() { /* TODO */ }
void Cpu::op_cb_24() { /* TODO */ }
void Cpu::op_cb_25() { /* TODO */ }
void Cpu::op_cb_26() { /* TODO */ }
void Cpu::op_cb_27() { /* TODO */ }
void Cpu::op_cb_28() { /* TODO */ }
void Cpu::op_cb_29() { /* TODO */ }
void Cpu::op_cb_2a() { /* TODO */ }
void Cpu::op_cb_2b() { /* TODO */ }
void Cpu::op_cb_2c() { /* TODO */ }
void Cpu::op_cb_2d() { /* TODO */ }
void Cpu::op_cb_2e() { /* TODO */ }
void Cpu::op_cb_2f() { /* TODO */ }
void Cpu::op_cb_30() { /* TODO */ }
void Cpu::op_cb_31() { /* TODO */ }
void Cpu::op_cb_32() { /* TODO */ }
void Cpu::op_cb_33() { /* TODO */ }
void Cpu::op_cb_34() { /* TODO */ }
void Cpu::op_cb_35() { /* TODO */ }
void Cpu::op_cb_36() { /* TODO */ }
void Cpu::op_cb_37() { /* TODO */ }
void Cpu::op_cb_38() { /* TODO */ }
void Cpu::op_cb_39() { /* TODO */ }
void Cpu::op_cb_3a() { /* TODO */ }
void Cpu::op_cb_3b() { /* TODO */ }
void Cpu::op_cb_3c() { /* TODO */ }
void Cpu::op_cb_3d() { /* TODO */ }
void Cpu::op_cb_3e() { /* TODO */ }
void Cpu::op_cb_3f() { /* TODO */ }
void Cpu::op_cb_40() { /* TODO */ }
void Cpu::op_cb_41() { /* TODO */ }
void Cpu::op_cb_42() { /* TODO */ }
void Cpu::op_cb_43() { /* TODO */ }
void Cpu::op_cb_44() { /* TODO */ }
void Cpu::op_cb_45() { /* TODO */ }
void Cpu::op_cb_46() { /* TODO */ }
void Cpu::op_cb_47() { /* TODO */ }
void Cpu::op_cb_48() { /* TODO */ }
void Cpu::op_cb_49() { /* TODO */ }
void Cpu::op_cb_4a() { /* TODO */ }
void Cpu::op_cb_4b() { /* TODO */ }
void Cpu::op_cb_4c() { /* TODO */ }
void Cpu::op_cb_4d() { /* TODO */ }
void Cpu::op_cb_4e() { /* TODO */ }
void Cpu::op_cb_4f() { /* TODO */ }
void Cpu::op_cb_50() { /* TODO */ }
void Cpu::op_cb_51() { /* TODO */ }
void Cpu::op_cb_52() { /* TODO */ }
void Cpu::op_cb_53() { /* TODO */ }
void Cpu::op_cb_54() { /* TODO */ }
void Cpu::op_cb_55() { /* TODO */ }
void Cpu::op_cb_56() { /* TODO */ }
void Cpu::op_cb_57() { /* TODO */ }
void Cpu::op_cb_58() { /* TODO */ }
void Cpu::op_cb_59() { /* TODO */ }
void Cpu::op_cb_5a() { /* TODO */ }
void Cpu::op_cb_5b() { /* TODO */ }
void Cpu::op_cb_5c() { /* TODO */ }
void Cpu::op_cb_5d() { /* TODO */ }
void Cpu::op_cb_5e() { /* TODO */ }
void Cpu::op_cb_5f() { /* TODO */ }
void Cpu::op_cb_60() { /* TODO */ }
void Cpu::op_cb_61() { /* TODO */ }
void Cpu::op_cb_62() { /* TODO */ }
void Cpu::op_cb_63() { /* TODO */ }
void Cpu::op_cb_64() { /* TODO */ }
void Cpu::op_cb_65() { /* TODO */ }
void Cpu::op_cb_66() { /* TODO */ }
void Cpu::op_cb_67() { /* TODO */ }
void Cpu::op_cb_68() { /* TODO */ }
void Cpu::op_cb_69() { /* TODO */ }
void Cpu::op_cb_6a() { /* TODO */ }
void Cpu::op_cb_6b() { /* TODO */ }
void Cpu::op_cb_6c() { /* TODO */ }
void Cpu::op_cb_6d() { /* TODO */ }
void Cpu::op_cb_6e() { /* TODO */ }
void Cpu::op_cb_6f() { /* TODO */ }
void Cpu::op_cb_70() { /* TODO */ }
void Cpu::op_cb_71() { /* TODO */ }
void Cpu::op_cb_72() { /* TODO */ }
void Cpu::op_cb_73() { /* TODO */ }
void Cpu::op_cb_74() { /* TODO */ }
void Cpu::op_cb_75() { /* TODO */ }
void Cpu::op_cb_76() { /* TODO */ }
void Cpu::op_cb_77() { /* TODO */ }
void Cpu::op_cb_78() { /* TODO */ }
void Cpu::op_cb_79() { /* TODO */ }
void Cpu::op_cb_7a() { /* TODO */ }
void Cpu::op_cb_7b() { /* TODO */ }
void Cpu::op_cb_7c() { /* TODO */ }
void Cpu::op_cb_7d() { /* TODO */ }
void Cpu::op_cb_7e() { /* TODO */ }
void Cpu::op_cb_7f() { /* TODO */ }
void Cpu::op_cb_80() { /* TODO */ }
void Cpu::op_cb_81() { /* TODO */ }
void Cpu::op_cb_82() { /* TODO */ }
void Cpu::op_cb_83() { /* TODO */ }
void Cpu::op_cb_84() { /* TODO */ }
void Cpu::op_cb_85() { /* TODO */ }
void Cpu::op_cb_86() { /* TODO */ }
void Cpu::op_cb_87() { /* TODO */ }
void Cpu::op_cb_88() { /* TODO */ }
void Cpu::op_cb_89() { /* TODO */ }
void Cpu::op_cb_8a() { /* TODO */ }
void Cpu::op_cb_8b() { /* TODO */ }
void Cpu::op_cb_8c() { /* TODO */ }
void Cpu::op_cb_8d() { /* TODO */ }
void Cpu::op_cb_8e() { /* TODO */ }
void Cpu::op_cb_8f() { /* TODO */ }
void Cpu::op_cb_90() { /* TODO */ }
void Cpu::op_cb_91() { /* TODO */ }
void Cpu::op_cb_92() { /* TODO */ }
void Cpu::op_cb_93() { /* TODO */ }
void Cpu::op_cb_94() { /* TODO */ }
void Cpu::op_cb_95() { /* TODO */ }
void Cpu::op_cb_96() { /* TODO */ }
void Cpu::op_cb_97() { /* TODO */ }
void Cpu::op_cb_98() { /* TODO */ }
void Cpu::op_cb_99() { /* TODO */ }
void Cpu::op_cb_9a() { /* TODO */ }
void Cpu::op_cb_9b() { /* TODO */ }
void Cpu::op_cb_9c() { /* TODO */ }
void Cpu::op_cb_9d() { /* TODO */ }
void Cpu::op_cb_9e() { /* TODO */ }
void Cpu::op_cb_9f() { /* TODO */ }
void Cpu::op_cb_a0() { /* TODO */ }
void Cpu::op_cb_a1() { /* TODO */ }
void Cpu::op_cb_a2() { /* TODO */ }
void Cpu::op_cb_a3() { /* TODO */ }
void Cpu::op_cb_a4() { /* TODO */ }
void Cpu::op_cb_a5() { /* TODO */ }
void Cpu::op_cb_a6() { /* TODO */ }
void Cpu::op_cb_a7() { /* TODO */ }
void Cpu::op_cb_a8() { /* TODO */ }
void Cpu::op_cb_a9() { /* TODO */ }
void Cpu::op_cb_aa() { /* TODO */ }
void Cpu::op_cb_ab() { /* TODO */ }
void Cpu::op_cb_ac() { /* TODO */ }
void Cpu::op_cb_ad() { /* TODO */ }
void Cpu::op_cb_ae() { /* TODO */ }
void Cpu::op_cb_af() { /* TODO */ }
void Cpu::op_cb_b0() { /* TODO */ }
void Cpu::op_cb_b1() { /* TODO */ }
void Cpu::op_cb_b2() { /* TODO */ }
void Cpu::op_cb_b3() { /* TODO */ }
void Cpu::op_cb_b4() { /* TODO */ }
void Cpu::op_cb_b5() { /* TODO */ }
void Cpu::op_cb_b6() { /* TODO */ }
void Cpu::op_cb_b7() { /* TODO */ }
void Cpu::op_cb_b8() { /* TODO */ }
void Cpu::op_cb_b9() { /* TODO */ }
void Cpu::op_cb_ba() { /* TODO */ }
void Cpu::op_cb_bb() { /* TODO */ }
void Cpu::op_cb_bc() { /* TODO */ }
void Cpu::op_cb_bd() { /* TODO */ }
void Cpu::op_cb_be() { /* TODO */ }
void Cpu::op_cb_bf() { /* TODO */ }
void Cpu::op_cb_c0() { /* TODO */ }
void Cpu::op_cb_c1() { /* TODO */ }
void Cpu::op_cb_c2() { /* TODO */ }
void Cpu::op_cb_c3() { /* TODO */ }
void Cpu::op_cb_c4() { /* TODO */ }
void Cpu::op_cb_c5() { /* TODO */ }
void Cpu::op_cb_c6() { /* TODO */ }
void Cpu::op_cb_c7() { /* TODO */ }
void Cpu::op_cb_c8() { /* TODO */ }
void Cpu::op_cb_c9() { /* TODO */ }
void Cpu::op_cb_ca() { /* TODO */ }
void Cpu::op_cb_cb() { /* TODO */ }
void Cpu::op_cb_cc() { /* TODO */ }
void Cpu::op_cb_cd() { /* TODO */ }
void Cpu::op_cb_ce() { /* TODO */ }
void Cpu::op_cb_cf() { /* TODO */ }
void Cpu::op_cb_d0() { /* TODO */ }
void Cpu::op_cb_d1() { /* TODO */ }
void Cpu::op_cb_d2() { /* TODO */ }
void Cpu::op_cb_d3() { /* TODO */ }
void Cpu::op_cb_d4() { /* TODO */ }
void Cpu::op_cb_d5() { /* TODO */ }
void Cpu::op_cb_d6() { /* TODO */ }
void Cpu::op_cb_d7() { /* TODO */ }
void Cpu::op_cb_d8() { /* TODO */ }
void Cpu::op_cb_d9() { /* TODO */ }
void Cpu::op_cb_da() { /* TODO */ }
void Cpu::op_cb_db() { /* TODO */ }
void Cpu::op_cb_dc() { /* TODO */ }
void Cpu::op_cb_dd() { /* TODO */ }
void Cpu::op_cb_de() { /* TODO */ }
void Cpu::op_cb_df() { /* TODO */ }
void Cpu::op_cb_e0() { /* TODO */ }
void Cpu::op_cb_e1() { /* TODO */ }
void Cpu::op_cb_e2() { /* TODO */ }
void Cpu::op_cb_e3() { /* TODO */ }
void Cpu::op_cb_e4() { /* TODO */ }
void Cpu::op_cb_e5() { /* TODO */ }
void Cpu::op_cb_e6() { /* TODO */ }
void Cpu::op_cb_e7() { /* TODO */ }
void Cpu::op_cb_e8() { /* TODO */ }
void Cpu::op_cb_e9() { /* TODO */ }
void Cpu::op_cb_ea() { /* TODO */ }
void Cpu::op_cb_eb() { /* TODO */ }
void Cpu::op_cb_ec() { /* TODO */ }
void Cpu::op_cb_ed() { /* TODO */ }
void Cpu::op_cb_ee() { /* TODO */ }
void Cpu::op_cb_ef() { /* TODO */ }
void Cpu::op_cb_f0() { /* TODO */ }
void Cpu::op_cb_f1() { /* TODO */ }
void Cpu::op_cb_f2() { /* TODO */ }
void Cpu::op_cb_f3() { /* TODO */ }
void Cpu::op_cb_f4() { /* TODO */ }
void Cpu::op_cb_f5() { /* TODO */ }
void Cpu::op_cb_f6() { /* TODO */ }
void Cpu::op_cb_f7() { /* TODO */ }
void Cpu::op_cb_f8() { /* TODO */ }
void Cpu::op_cb_f9() { /* TODO */ }
void Cpu::op_cb_fa() { /* TODO */ }
void Cpu::op_cb_fb() { /* TODO */ }
void Cpu::op_cb_fc() { /* TODO */ }
void Cpu::op_cb_fd() { /* TODO */ }
void Cpu::op_cb_fe() { /* TODO */ }
void Cpu::op_cb_ff() { /* TODO */ }
/* clang-format on */

} // namespace gbc
