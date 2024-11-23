#ifndef __LIB_ISA_INCLUDE_SIM_ISA_HH__
#define __LIB_ISA_INCLUDE_SIM_ISA_HH__

#include <cstdint>

namespace sim::isa {

// Useful typedefs
using Word = std::uint32_t;
using Addr = std::uint32_t;

constexpr std::size_t kNumRegisters = 32;

/**
 * ISA description
 * 4 bytes for each instruction
 * first byte - opcode
 * second byte - dest (if present)
 * third, fourth - two sources (if present)
 */
enum class Opcode : std::uint8_t {
  kUnknown = 0,
  kAdd,
  kHalt,
  kJump,
  kLoad,
  kStore,
  kBeq,
};

constexpr Opcode get_opcode(Word bytes) {
  // Get highest byte
  return static_cast<Opcode>((bytes >> 24U) & 0xFFU);
}
constexpr Word get_dst(Word bytes) { return (bytes >> 16U) & 0xFFU; }
constexpr Word get_src1(Word bytes) { return (bytes >> 8U) & 0xFFU; }
constexpr Word get_src2(Word bytes) { return bytes & 0xFFU; }

struct Instruction {
  Opcode opc{};
  Word src1{}, src2{}, dst{};
};
} // namespace sim::isa

#endif // __LIB_ISA_INCLUDE_SIM_ISA_HH__
