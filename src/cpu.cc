#include "cpu.hh"
#include "log.hh"
#include "mmu.hh"
#include "opcode_cycles.hh"
#include "opcode_names.hh"
#include "register.hh"
#include "util.hh"

#include <cstdint>
#include <iostream>
#include <sstream>

namespace bugme {

Cpu::Cpu(std::shared_ptr<Mmu> mmu)
    : af(a, f), bc(b, c), de(d, e), hl(h, l), mmu_(mmu),
      interrupt_enable(mmu->addr(0xFFFF)), interrupt_flag(mmu->addr(0xFF0F)) {
  reset();
}

mcycles_t Cpu::tick() {
  check_interrupts();

  if (halted_ || stopped_) {
    return 1;
  }

  //word_t old_pc = pc.value();
  byte_t opcode = mmu_->read(pc.value());
  next_byte();
  if (opcode == 0x00) {
    // log_error("received unexpected nop at 0x%x, exiting now", pc.value() -
    // 1);
    // exit(1);
  }
  if (opcode != 0xcb) {
    log_debug("0x%04X: %s (0x%x)", pc.value() - 1,
              opcode::NAMES[opcode].c_str(), opcode);
    op(opcode);
    mcycles_t cycles;
    if (did_branch_) {
      did_branch_ = false;
      cycles = opcode::BRANCHED_CYCLES[opcode];
    }
    cycles = opcode::CYCLES[opcode];
    /*if (old_pc == pc.value() && opcode != 0x00) {
      log_error("Upcoming tight loop detected. Exiting.");
      exit(2);
    }*/
    return cycles;
  } else {
    opcode = next_byte();
    log_debug("0x%04X: %s (0xcb 0x%x)", pc.value() - 2,
              opcode::CB_NAMES[opcode].c_str(), opcode);
    cb_op(opcode);
    return opcode::CB_CYCLES[opcode];
  }
}

void Cpu::reset() {
  mmu_->reset();

  a.reset();
  b.reset();
  c.reset();
  d.reset();
  e.reset();
  f.reset();
  h.reset();
  l.reset();
  sp.reset();
  pc.reset();
  interrupt_master_enable = false;

  stopped_ = false;
  halted_ = false;
  did_branch_ = false;
}

void Cpu::int_vblank() { interrupt_flag.set_bit(0); }

void Cpu::int_lcdc() { interrupt_flag.set_bit(1); }

void Cpu::int_timer() { interrupt_flag.set_bit(2); }

void Cpu::int_serial() { interrupt_flag.set_bit(3); }

void Cpu::int_joypad() { interrupt_flag.set_bit(4); }

void Cpu::check_interrupts() {
  byte_t fired_interrupts = interrupt_flag.value() & interrupt_enable.value();
  if (!fired_interrupts) {
    return;
  }

  if (interrupt_master_enable) {
    push(pc);

    if ((fired_interrupts >> 0) & 1) {
      interrupt_flag.clear_bit(0);
      pc.set(interrupt::VBLANK);
      interrupt_master_enable = false;
    } else if ((fired_interrupts >> 1) & 1) {
      interrupt_flag.clear_bit(1);
      pc.set(interrupt::LCDC_STATUS);
      interrupt_master_enable = false;
    } else if ((fired_interrupts >> 2) & 1) {
      interrupt_flag.clear_bit(2);
      pc.set(interrupt::TIMER);
      interrupt_master_enable = false;
    } else if ((fired_interrupts >> 3) & 1) {
      pc.set(interrupt::SERIAL);
      interrupt_flag.clear_bit(3);
      interrupt_master_enable = false;
    } else if ((fired_interrupts >> 4) & 1) {
      pc.set(interrupt::JOYPAD);
      interrupt_flag.clear_bit(4);
      interrupt_master_enable = false;
    }
  } else if (halted_) {
    halt_bug_no_step_mode_ = true;
  }

  halted_ = false; // unhalt now that we found an interrupt
}

byte_t Cpu::next_byte() {
  byte_t byte = mmu_->read(pc.value());
  if (halt_bug_no_step_mode_) {
    halt_bug_no_step_mode_ = false;
    return byte;
  }
  pc.increment();
  return byte;
}

word_t Cpu::next_word() {
  word_t word = util::fuse(mmu_->read(pc.value() + 1), mmu_->read(pc.value()));
  pc.increment();
  pc.increment();
  return word;
}

} // namespace bugme
