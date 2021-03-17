#ifndef GBC_GBC_H
#define GBC_GBC_H

#include <memory>
#include <string>

namespace gbc {

class Cpu;
class Mmu;

class Gbc {
public:
  Gbc();

  void start();
  std::string status() const;

private:
  std::shared_ptr<Mmu> mmu;
  std::shared_ptr<Cpu> cpu;
};
} // namespace gbc
#endif
