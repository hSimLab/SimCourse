#include "sim/decoder.hh"

#include <cassert>

namespace sim::decoder {
isa::Instruction decode(isa::Word bytes) {
  isa::Instruction insn{.opc = isa::get_opcode(bytes)};

  switch (insn.opc) {
  case isa::Opcode::kAdd:
  case isa::Opcode::kHalt:
  case isa::Opcode::kJump:
  case isa::Opcode::kLoad:
  case isa::Opcode::kStore:
  case isa::Opcode::kBeq:
    insn.dst = isa::get_dst(bytes);
    insn.src1 = isa::get_src1(bytes);
    insn.src2 = isa::get_src2(bytes);
    break;
  default:
    assert(false && "Unknown instruction");
  }

  return insn;
}
} // namespace sim::decoder
