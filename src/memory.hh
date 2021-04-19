#ifndef BUGME_MEMORY_H
#define BUGME_MEMORY_H

#include "types.hh"
#include <vector>

namespace bugme {

class Memory : public Noncopyable {
public:
  Memory();
  virtual ~Memory() = default;

  byte_t read(word_t addr) const;
  void write(word_t addr, byte_t byte);

private:
  std::vector<byte_t> memory_;
};

} // namespace bugme

#endif
