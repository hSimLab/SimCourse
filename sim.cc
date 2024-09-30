#include <cassert>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <vector>

using Register = std::uint32_t;
using Addr = std::uint32_t;
constexpr std::size_t kNumRegisters = 32;
constexpr std::size_t kMemSize = 0x400000;
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

class Memory {
  std::vector<Register> data;

public:
  Memory() : data(kMemSize) {}

  Register load(Addr addr) const { return data[addr]; }

  void store(Addr addr, Register value) { data[addr] = value; }
};

struct CpuState {
  Register pc{};
  Register regs[kNumRegisters]{};
  Memory *memory{};
  bool finished{false};

  explicit CpuState(Memory *mem) : memory(mem) {}

  Register getReg(std::size_t id) const { return regs[id]; }

  void setReg(std::size_t id, Register value) { regs[id] = value; }

  void dump() const {
    std::cout << "CpuState dump: \n";
    std::cout << "PC: " << pc << std::endl;
    for (size_t i = 0; i < kNumRegisters; ++i) {
      std::cout << "[" << i << "] = " << regs[i] << std::endl;
    }
  }
};

struct Instruction {
  Opcode opc{};
  Register src1{}, src2{}, dst{};
};

Register fetch(CpuState *cpu) { return cpu->memory->load(cpu->pc); }

Opcode get_opcode(Register bytes) {
  // Get highest byte
  return static_cast<Opcode>((bytes >> 24U) & 0xFFU);
}
Register get_dst(Register bytes) { return (bytes >> 16U) & 0xFFU; }
Register get_src1(Register bytes) { return (bytes >> 8U) & 0xFFU; }
Register get_src2(Register bytes) {
  // Get lowest byte
  return bytes & 0xFFU;
}

Instruction decode(Register bytes) {
  Instruction insn{};
  insn.opc = get_opcode(bytes);
  switch (insn.opc) {
  case Opcode::kAdd:
  case Opcode::kHalt:
  case Opcode::kJump:
  case Opcode::kLoad:
  case Opcode::kStore:
  case Opcode::kBeq:
    insn.dst = get_dst(bytes);
    insn.src1 = get_src1(bytes);
    insn.src2 = get_src2(bytes);
    break;
  default:
    assert(false && "Unknown instruction");
  }

  return insn;
}

void execute(CpuState *cpu, Instruction insn) {
  switch (insn.opc) {
  case Opcode::kAdd: {
    auto res = cpu->getReg(insn.src1) + cpu->getReg(insn.src2);
    cpu->setReg(insn.dst, res);
    cpu->pc += 1;
    break;
  }
  case Opcode::kHalt: {
    cpu->finished = true;
    break;
  }
  case Opcode::kJump: {
    cpu->pc = cpu->getReg(insn.dst);
    break;
  }
  case Opcode::kLoad: {
    auto data = cpu->memory->load(cpu->getReg(insn.src1));
    cpu->setReg(insn.dst, data);
    cpu->pc += 1;
    break;
  }
  case Opcode::kStore: {
    auto data = cpu->getReg(insn.dst);
    cpu->memory->store(cpu->getReg(insn.src1), data);
    cpu->pc += 1;
    break;
  }
  case Opcode::kBeq: {
    bool eq = cpu->getReg(insn.src1) == cpu->getReg(insn.src2);
    cpu->pc = eq ? cpu->getReg(insn.dst) : cpu->pc + 1;
    break;
  }
  default:
    assert(false && "Unknown instruction");
  }
}

int main() {

  // Define program
  std::uint32_t program[] = {
      0x01010203, // Add x1, x2, x3
      0x02000000, // Halt
  };

  Memory mem;

  // Define entry point
  std::size_t entryAddr = 42;

  // Load program to memory
  for (size_t i = 0; i < std::size(program); ++i) {
    mem.store(entryAddr + i, program[i]);
  }

  // Init cpu state
  CpuState cpu{&mem};

  // set entry point
  cpu.pc = entryAddr;

  // init regfile
  cpu.regs[2] = 20;
  cpu.regs[3] = 40;

  // main loop
  while (!cpu.finished) {
    auto bytes = fetch(&cpu);
    Instruction insn = decode(bytes);

    execute(&cpu, insn);
  }

  std::cout << "Done execution" << std::endl;
  cpu.dump();
}
