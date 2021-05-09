#include "cpu.hh"

#include "bootrom.hh"
#include "cartridge.hh"
#include "joypad.hh"
#include "log.hh"
#include "memory.hh"
#include "mmap.hh"
#include "opcode_cycles.hh"
#include "opcode_names.hh"
#include "ppu.hh"
#include "register.hh"
#include "timer.hh"
#include "util.hh"

#include <cstdint>
#include <iostream>
#include <sstream>

namespace bugme {

Cpu::Cpu(Memory &memory, Cartridge &cartridge, PpuBus &ppuBus,
         TimerBus &timerBus, JoypadBus &joypadBus)
    : memory_(memory), cartridge_(cartridge), ppuBus_(ppuBus),
      timerBus_(timerBus), joypadBus_(joypadBus), af(a, f), bc(b, c), de(d, e),
      hl(h, l) {
  reset();
  ppuBus_.register_vblank_interrupt_request_cb(
      [&]() { interrupt_flag.set_vblank_interrupt_request(); });
  ppuBus_.register_lcd_stat_interrupt_request_cb(
      [&]() { interrupt_flag.set_lcd_stat_interrupt_request(); });
  timerBus_.register_timer_interrupt_request_cb(
      [&]() { interrupt_flag.set_timer_interrupt_request(); });
  joypadBus_.register_joypad_interrupt_request_cb(
      [&]() { interrupt_flag.set_joypad_interrupt_request(); });
}

byte_t Cpu::read_(word_t addr) const {
  // cartridge rom
  if (util::in_range(addr, mmap::CARTRIDGE_ROM_START,
                     mmap::CARTRIDGE_ROM_END)) {
    if (util::in_range(addr, mmap::BOOT_ROM_START, mmap::BOOT_ROM_END) &&
        boot_rom_control.value() == 0x0) {
      return boot::ROM[addr];
    }

    return cartridge_.read(addr);
  }

  // vram
  if (util::in_range(addr, mmap::VRAM_START, mmap::VRAM_END)) {
    return ppuBus_.vram.at(addr - mmap::VRAM_START);
  }

  // cartridge ram
  if (util::in_range(addr, mmap::CARTRIDGE_RAM_START,
                     mmap::CARTRIDGE_RAM_END)) {
    return memory_.read(addr);
  }

  // work ram
  if (util::in_range(addr, mmap::WORK_RAM_START, mmap::WORK_RAM_END)) {
    return memory_.read(addr);
  }

  // echo work ram
  if (util::in_range(addr, mmap::ECHO_WORK_RAM_START,
                     mmap::ECHO_WORK_RAM_END)) {
    log_warn("reading from echo ram [%x]", addr);
    return memory_.read(addr - 0x2000);
  }

  // oam
  if (util::in_range(addr, mmap::OAM_START, mmap::OAM_END)) {
    return ppuBus_.oam.at(addr - mmap::OAM_START);
  }

  // unused
  if (util::in_range(addr, mmap::UNUSED_START, mmap::UNUSED_END)) {
    // log_warn("reading from unused ram [%x]", addr);
    return memory_.read(addr);
  }

  // i/o registers
  if (util::in_range(addr, mmap::IO_REGISTERS_START, mmap::IO_REGISTERS_END)) {
    switch (addr) {
    case mmap::joypad::JOYP:
      return joypadBus_.joyp.value();

    case mmap::timer::DIV:
      return timerBus_.divider.value();
    case mmap::timer::TIMA:
      return timerBus_.timer_counter.value();
    case mmap::timer::TMA:
      return timerBus_.timer_modulo.value();
    case mmap::timer::TAC:
      return timerBus_.timer_control.value();

    case mmap::INTERRUPTS_FLAG:
      return interrupt_flag.value();

    case mmap::ppu::LCD_CONTROL:
      return ppuBus_.lcd_control.value();
    case mmap::ppu::LCD_STATUS:
      return ppuBus_.lcd_status.value();
    case mmap::ppu::SCROLL_Y:
      return ppuBus_.scroll_y.value();
    case mmap::ppu::SCROLL_X:
      return ppuBus_.scroll_x.value();
    case mmap::ppu::LINE:
      return ppuBus_.line.value();
    case mmap::ppu::LY_COMPARE:
      return ppuBus_.ly_compare.value();
    case mmap::ppu::DMA_TRANSFER:
      return ppuBus_.dma_transfer.value();
    case mmap::ppu::BG_PALETTE:
      return ppuBus_.bg_palette.value();
    case mmap::ppu::SPRITE_PALETTE_0:
      return ppuBus_.sprite_palette_0.value();
    case mmap::ppu::SPRITE_PALETTE_1:
      return ppuBus_.sprite_palette_1.value();
    case mmap::ppu::WINDOW_Y:
      return ppuBus_.window_y.value();
    case mmap::ppu::WINDOW_X:
      return ppuBus_.window_x.value();

    case mmap::BOOT_ROM_CONTROL:
      return boot_rom_control.value();

    default:
      return memory_.read(addr);
    }
  }

  // zero page
  if (util::in_range(addr, mmap::ZERO_PAGE_START, mmap::ZERO_PAGE_END)) {
    return memory_.read(addr);
  }

  if (addr == mmap::INTERRUPTS_ENABLED) {
    return interrupt_enable.value();
  }

  log_error("could not read from address 0x%x", addr);
  return 0x0;
}

void Cpu::write_(word_t addr, byte_t byte) {
  // cartridge rom
  if (util::in_range(addr, mmap::CARTRIDGE_ROM_START,
                     mmap::CARTRIDGE_ROM_END)) {
    memory_.write(addr, byte);
    return;
  }

  // vram
  if (util::in_range(addr, mmap::VRAM_START, mmap::VRAM_END)) {
    // TODO: NOT ALWAYS!!!!
    ppuBus_.vram.at(addr - mmap::VRAM_START) = byte;
    return;
  }

  // cartridge ram
  if (util::in_range(addr, mmap::CARTRIDGE_RAM_START,
                     mmap::CARTRIDGE_RAM_END)) {
    memory_.write(addr, byte);
    return;
  }

  // work ram
  if (util::in_range(addr, mmap::WORK_RAM_START, mmap::WORK_RAM_END)) {
    memory_.write(addr, byte);
    return;
  }

  // echo work ram
  if (util::in_range(addr, mmap::ECHO_WORK_RAM_START,
                     mmap::ECHO_WORK_RAM_END)) {
    log_warn("writing to echo ram [%x]", addr);
    memory_.write(addr - 0x2000, byte);
    return;
  }

  // oam
  if (util::in_range(addr, mmap::OAM_START, mmap::OAM_END)) {
    ppuBus_.oam.at(addr - mmap::OAM_START) = byte;
    return;
  }

  // unused
  if (util::in_range(addr, mmap::UNUSED_START, mmap::UNUSED_END)) {
    // log_warn("writing to unused ram [%x]", addr);
    memory_.write(addr, byte);
    return;
  }

  // i/o registers
  if (util::in_range(addr, mmap::IO_REGISTERS_START, mmap::IO_REGISTERS_END)) {
    switch (addr) {
    case mmap::joypad::JOYP:
      // hack to make sure that no buttons are pressed?
      joypadBus_.joyp.set(byte | 0b1111);
      return;

    case mmap::timer::DIV:
      // DIV register -- writes 0 on attempt
      timerBus_.divider.set(0);
      return;
    case mmap::timer::TIMA:
      timerBus_.timer_counter.set(byte);
      return;
    case mmap::timer::TMA:
      timerBus_.timer_modulo.set(byte);
      return;
    case mmap::timer::TAC:
      timerBus_.timer_control.set(byte);
      return;

    case mmap::INTERRUPTS_FLAG:
      interrupt_flag.set(byte);
      return;

    case mmap::ppu::LCD_CONTROL:
      ppuBus_.lcd_control.set(byte);
      return;
    case mmap::ppu::LCD_STATUS:
      ppuBus_.lcd_status.set(byte);
      return;
    case mmap::ppu::SCROLL_Y:
      ppuBus_.scroll_y.set(byte);
      return;
    case mmap::ppu::SCROLL_X:
      ppuBus_.scroll_x.set(byte);
      return;
    case mmap::ppu::LINE:
      ppuBus_.line.set(byte);
      return;
    case mmap::ppu::LY_COMPARE:
      ppuBus_.ly_compare.set(byte);
      return;
    case mmap::ppu::DMA_TRANSFER:
      ppuBus_.dma_transfer.set(byte);
      dma_transfer_(byte);
      return;
    case mmap::ppu::BG_PALETTE:
      ppuBus_.bg_palette.set(byte);
      return;
    case mmap::ppu::SPRITE_PALETTE_0:
      ppuBus_.sprite_palette_0.set(byte);
      return;
    case mmap::ppu::SPRITE_PALETTE_1:
      ppuBus_.sprite_palette_1.set(byte);
      return;
    case mmap::ppu::WINDOW_Y:
      ppuBus_.window_y.set(byte);
      return;
    case mmap::ppu::WINDOW_X:
      ppuBus_.window_x.set(byte);
      return;

    case mmap::BOOT_ROM_CONTROL:
      boot_rom_control.set(byte);
      return;

    default:
      memory_.write(addr, byte);
      return;
    }
  }

  // zero page
  if (util::in_range(addr, mmap::ZERO_PAGE_START, mmap::ZERO_PAGE_END)) {
    memory_.write(addr, byte);
    return;
  }

  if (addr == mmap::INTERRUPTS_ENABLED) {
    interrupt_enable.set(byte);
    return;
  }

  log_error("could not write to address 0x%x", addr);
  return;
}

void Cpu::dma_transfer_(byte_t byte) {
  word_t source_base = static_cast<word_t>(byte) << 8;
  for (word_t offset = 0; offset <= 0x009F; ++offset) {
    ppuBus_.oam.at(offset) = read_(source_base + offset);
  }
}

mcycles_t Cpu::tick() {
  check_interrupts();

  if (halted_ || stopped_) {
    return 1;
  }

  // word_t old_pc = pc.value();
  byte_t opcode = read_(pc.value());
  next_byte();
  if (opcode == 0x00) {
    // log_error("received unexpected nop at 0x%x, exiting now", pc.value() -
    // 1);
    // exit(1);
  }
  if (opcode != 0xcb) {
    log_debug("[cpu] 0x%04X: %s (0x%x)", pc.value() - 1,
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
    log_debug("[cpu] 0x%04X: %s (0xcb 0x%x)", pc.value() - 2,
              opcode::CB_NAMES[opcode].c_str(), opcode);
    cb_op(opcode);
    return opcode::CB_CYCLES[opcode];
  }
}

void Cpu::reset() {
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

void Cpu::check_interrupts() {
  byte_t fired_interrupts = interrupt_flag.value() & interrupt_enable.value();
  if (!fired_interrupts) {
    return;
  }

  if (interrupt_master_enable) {
    push(pc);

    if ((fired_interrupts >> 0) & 1) {
      interrupt_flag.clear_bit(0);
      pc.set(interrupt_vectors::VBLANK);
      interrupt_master_enable = false;
    } else if ((fired_interrupts >> 1) & 1) {
      interrupt_flag.clear_bit(1);
      pc.set(interrupt_vectors::LCDC_STATUS);
      interrupt_master_enable = false;
    } else if ((fired_interrupts >> 2) & 1) {
      interrupt_flag.clear_bit(2);
      pc.set(interrupt_vectors::TIMER);
      interrupt_master_enable = false;
    } else if ((fired_interrupts >> 3) & 1) {
      pc.set(interrupt_vectors::SERIAL);
      interrupt_flag.clear_bit(3);
      interrupt_master_enable = false;
    } else if ((fired_interrupts >> 4) & 1) {
      pc.set(interrupt_vectors::JOYPAD);
      interrupt_flag.clear_bit(4);
      interrupt_master_enable = false;
    }
  } else if (halted_) {
    halt_bug_no_step_mode_ = true;
  }

  halted_ = false; // unhalt now that we found an interrupt
}

byte_t Cpu::next_byte() {
  byte_t byte = read_(pc.value());
  if (halt_bug_no_step_mode_) {
    halt_bug_no_step_mode_ = false;
    return byte;
  }
  pc.increment();
  return byte;
}

word_t Cpu::next_word() {
  word_t word = util::fuse(read_(pc.value() + 1), read_(pc.value()));
  pc.increment();
  pc.increment();
  return word;
}

} // namespace bugme
