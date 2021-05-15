#ifndef BUGME_BUS_HH
#define BUGME_BUS_HH

#include "types.hh"

namespace bugme {

/**
 * A wrapper which represents a cross-component interface.
 *
 * Each component of the emulator (joypad, ppu, apu, etc) is allocated on the
 * stack, allowing us to not deal with smart pointers. Therefore, to facilitate
 * cross-component interfaces, we just pass references to each component and
 * allow them to interface directly.
 *
 * As such, a Bus allows a component (again, such as the joypad, ppu, apu, etc)
 * to define its own interface to other components (chiefly, the CPU is the
 * client, however). I took this approach because it seems to reflect how the
 * physical Gameboy works, where memory is mapped to registers on the individual
 * components themselves (for example, the Ppu class holds all of its relevant
 * registers, and the Cpu class knows to access the Ppu bus to access them).
 */
template <class T> struct Bus : public Noncopyable {};

} // namespace bugme

#endif
