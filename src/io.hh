#ifndef BUGME_IO_H
#define BUGME_IO_H

#include "interrupts.hh"
#include "register.hh"

#include <functional>

namespace bugme {

template <class T> struct Io : public Noncopyable { friend T; };

} // namespace bugme

#endif
