#ifndef __LIB_MEMORY_INCLUDE_SIM_MEMORY_HH__
#define __LIB_MEMORY_INCLUDE_SIM_MEMORY_HH__

#include <vector>

#include "sim/isa.hh"

namespace sim {
class Memory {
  std::vector<isa::Word> data;

public:
  static constexpr std::size_t kMemSize = 0x400000;

  Memory() : data(kMemSize) {}

  isa::Word load(isa::Addr addr) const { return data[addr]; }

  void store(isa::Addr addr, isa::Word value) { data[addr] = value; }

  auto &get_data() { return data; }
};
} // namespace sim

#endif // __LIB_MEMORY_INCLUDE_SIM_MEMORY_HH__
