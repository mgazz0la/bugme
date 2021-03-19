#include "timer.hh"

#include "mmu.hh"

namespace gbc {

Timer::Timer(std::shared_ptr<Mmu> mmu)
    : mmu_(mmu), divider(mmu->addr(0xFF04)), timer_counter(mmu->addr(0xFF05)),
      timer_modulo(mmu->addr(0xFF06)), timer_control(mmu->addr(0xFF07)) {}

} // namespace gbc
