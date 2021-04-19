#ifndef BUGME_BUS_H
#define BUGME_BUS_H

#include "types.hh"

namespace bugme {

template <class T> struct Bus : public Noncopyable { friend T; };

} // namespace bugme

#endif
