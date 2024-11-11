#include "asmjit/core/compiler.h"
#include "asmjit/core/func.h"
#include "asmjit/core/logger.h"
#include "asmjit/x86/x86compiler.h"
#include "asmjit/x86/x86operand.h"
#include <asmjit/asmjit.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <unordered_map>
#include <vector>

using Register = std::uint32_t;
using Addr = std::uint32_t;
constexpr std::size_t kNumRegisters = 32;
constexpr std::size_t kMemSize = 0x400000;
const std::size_t KBbThreshold = 10;
const std::size_t kAverageBbSize = 10;

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

struct Instruction {
  Opcode opc{};
  Register src1{}, src2{}, dst{};
};

class Memory {
  std::vector<Register> m_data;

public:
  Memory() : m_data(kMemSize) {}

  Register load(Addr addr) const { return m_data[addr]; }

  void store(Addr addr, Register value) { m_data[addr] = value; }

  std::vector<Register> &data() { return m_data; }
};

struct CpuState {
  Register pc{};
  Register regs[kNumRegisters]{};
  Memory *memory{};
  bool finished{false};

  //! NOTE: asmjit usage
  asmjit::JitRuntime runtime;
  using FuncTy = void (*)(void);

  std::unordered_map<Register, std::vector<Instruction>> bb_cache;
  std::unordered_map<Register, std::pair<FuncTy, size_t>> translated;

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

Register fetch(CpuState *cpu) { return cpu->memory->load(cpu->pc); }
Register fetch(CpuState *cpu, Register addr) { return cpu->memory->load(addr); }

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

bool is_terminate(Opcode opc) {
  switch (opc) {
  case Opcode::kJump:
  case Opcode::kBeq:
  case Opcode::kHalt:
    return true;
    break;
  default:
    return false;
    break;
  }
  return false;
}

std::vector<Instruction> lookup_bb(CpuState *cpu, Register addr) {
  auto find_res = cpu->bb_cache.find(addr);

  if (find_res == cpu->bb_cache.end()) {
    Register cur_addr = addr;
    std::vector<Instruction> bb;
    Instruction insn{};
    bb.reserve(kAverageBbSize);
    //
    do {
      auto bytes = fetch(cpu, cur_addr);
      insn = decode(bytes);
      bb.push_back(insn);
      cur_addr += 1;

    } while (!is_terminate(insn.opc));

    find_res = cpu->bb_cache.emplace(addr, bb).first;
  }

  return find_res->second;
}

// std::vector<Instruction> lookup_bb(CpuState *cpu, Register addr) {
//     if (!cpu->bb_cache.count(addr)) {
//         Register cur_addr = addr;
//         std::vector<Instruction> bb;
//         Instruction insn{};
//         bb.reserve(kAverageBbSize);
//         //
//         do {
//             auto bytes = fetch(cpu, cur_addr);
//             insn = decode(bytes);
//             bb.push_back(insn);
//             cur_addr += 1;

//         } while (!is_terminate(insn.opc));

//         cpu->bb_cache.emplace(addr, bb);
//     }

//     return cpu->bb_cache.at(addr);
// }

// helpers
static Register loadHelper(Memory *mem, Register addr) {
  return mem->load(addr);
}
static void storeHelper(Memory *mem, Register addr, Register value) {
  return mem->store(addr, value);
}

std::pair<CpuState::FuncTy, std::size_t>
translate(CpuState *cpu, const std::vector<Instruction> &bb) {
  std::size_t icount = 0;
  asmjit::CodeHolder code;
  code.init(cpu->runtime.environment());
  asmjit::FileLogger logger{stdout};
  // code.setLogger(&logger);

  asmjit::x86::Compiler cc{&code};
  cc.addFunc(asmjit::FuncSignature::build<void>());

  auto toDwordPtr = [&](auto &arg) {
    return asmjit::x86::dword_ptr((size_t)(&arg));
  };

  auto temp = cc.newGpd();
  auto temp2 = cc.newGpd();
  auto toRet = cc.newGpd();

  for (auto insn : bb) {
    switch (insn.opc) {
    case Opcode::kAdd: {
      cc.mov(temp, toDwordPtr(cpu->regs[insn.src1]));
      cc.mov(temp2, toDwordPtr(cpu->regs[insn.src2]));
      cc.add(temp, temp2);
      //
      cc.mov(asmjit::x86::dword_ptr((size_t)(&(cpu->regs[insn.dst]))), temp);

      cc.mov(temp, toDwordPtr(cpu->pc));
      cc.add(temp, 1);

      cc.mov(asmjit::x86::dword_ptr((size_t)(&cpu->pc)), temp);

      break;
    }
    case Opcode::kHalt: {
      cc.mov(asmjit::x86::byte_ptr((size_t)&cpu->finished), 1);
      cc.ret();
      break;
    }
    case Opcode::kJump: {
      cc.mov(temp, toDwordPtr(cpu->regs[insn.dst]));
      cc.mov(asmjit::x86::dword_ptr((size_t)(&cpu->pc)), temp);

      cc.ret();
      break;
    }
    case Opcode::kLoad: {
      asmjit::InvokeNode *invoke{};
      cc.mov(temp, toDwordPtr(cpu->regs[insn.src1]));

      cc.invoke(&invoke, (size_t)loadHelper,
                asmjit::FuncSignature::build<Register, Memory *, Register>());
      invoke->setArg(0, cpu->memory);
      invoke->setArg(1, temp);
      invoke->setRet(0, toRet);

      cc.mov(toDwordPtr(cpu->regs[insn.dst]), toRet);

      cc.mov(temp, toDwordPtr(cpu->pc));
      cc.add(temp, 1);
      cc.mov(asmjit::x86::dword_ptr((size_t)(&cpu->pc)), temp);
      break;
    }
    case Opcode::kStore: {
      cc.mov(temp, toDwordPtr(cpu->regs[insn.dst]));
      cc.mov(toRet, toDwordPtr(cpu->regs[insn.src1]));

      asmjit::InvokeNode *invoke{};
      cc.invoke(
          &invoke, (size_t)storeHelper,
          asmjit::FuncSignature::build<void, Memory *, Register, Register>());
      invoke->setArg(0, cpu->memory);
      invoke->setArg(1, toRet);
      invoke->setArg(2, temp);

      cc.mov(temp, toDwordPtr(cpu->pc));
      cc.add(temp, 1);

      cc.mov(asmjit::x86::dword_ptr((size_t)(&cpu->pc)), temp);

      break;
    }
    case Opcode::kBeq: {
      asmjit::Label beq_beg = cc.newLabel(), beq_end = cc.newLabel();

      cc.mov(temp, toDwordPtr(cpu->regs[insn.src1]));
      cc.mov(temp2, toDwordPtr(cpu->regs[insn.src2]));
      cc.cmp(temp, temp2);

      cc.je(beq_beg);

      cc.mov(temp, toDwordPtr(cpu->pc));
      cc.add(temp, 1);
      cc.jmp(beq_end);

      cc.bind(beq_beg);

      cc.mov(temp, toDwordPtr(cpu->regs[insn.dst]));

      cc.bind(beq_end);

      cc.mov(asmjit::x86::dword_ptr((size_t)(&cpu->pc)), temp);

      cc.ret();
      break;
    }
    default:
      assert(false && "Unknown instruction");
    }
  }

  cc.endFunc();
  cc.finalize();

  CpuState::FuncTy exec_func{};
  asmjit::Error err = cpu->runtime.add(&exec_func, &code);
  //
  if (err) {
    std::cout << "AsmJit failed: " << asmjit::DebugUtils::errorAsString(err)
              << std::endl;

    return {nullptr, icount};
  }

  return {exec_func, icount};
}

void interpret(CpuState *cpu, const std::vector<Instruction> &bb) {
  for (auto insn : bb) {
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
}

int main() {
  // Define program
  std::uint32_t program[] = {
#include "code.hpp"
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
#include "regfile.ini"

  // main loop
  while (!cpu.finished) {
    auto find_res = cpu.translated.find(cpu.pc);
    if (find_res != cpu.translated.end()) {
      find_res->second.first();
      continue;
    } else if (cpu.bb_cache.count(cpu.pc)) {
      auto &&basic_block = cpu.bb_cache[cpu.pc];
      if (basic_block.size() > KBbThreshold) {
        auto [jit_func, icount] = translate(&cpu, basic_block);

        if (jit_func) {
          cpu.translated[cpu.pc] = std::make_pair(jit_func, icount);
          jit_func();
          continue;
        }
      }
    }
    auto &&basic_block = lookup_bb(&cpu, cpu.pc);
    interpret(&cpu, basic_block);
  }

  cpu.dump();
  std::cout << "Done execution" << std::endl;
}
