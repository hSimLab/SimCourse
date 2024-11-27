#ifndef __LIB_CPU_STATE_INCLUDE_SIM_CPU_STATE_HH__
#define __LIB_CPU_STATE_INCLUDE_SIM_CPU_STATE_HH__

#include <cstddef>
#include <iostream>

#include "sim/isa.hh"

namespace sim {
class Memory;

struct CpuState {
  static constexpr std::size_t kNumRegisters = 32;

  isa::Word pc{};
  isa::Word regs[kNumRegisters]{};
  Memory *memory{};
  bool finished{false};

  explicit CpuState(Memory *mem) : memory(mem) {}

  isa::Word getReg(std::size_t id) const { return regs[id]; }

  void setReg(std::size_t id, isa::Word value) { regs[id] = value; }

  void dump(std::ostream &ost) const {
    ost << "CpuState dump: \n";
    ost << "PC: " << pc << std::endl;
    for (std::size_t i = 0; i < kNumRegisters; ++i) {
      ost << "[" << i << "] = " << regs[i] << std::endl;
    }
  }
};
} // namespace sim

#endif // __LIB_CPU_STATE_INCLUDE_SIM_CPU_STATE_HH__
