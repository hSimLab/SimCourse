#include <cassert>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <vector>

#include "sim/cpu_state.hh"
#include "sim/decoder.hh"
#include "sim/isa.hh"
#include "sim/memory.hh"

void execute(sim::CpuState *cpu, sim::isa::Instruction insn) {
  switch (insn.opc) {
  case sim::isa::Opcode::kAdd: {
    auto res = cpu->getReg(insn.src1) + cpu->getReg(insn.src2);
    cpu->setReg(insn.dst, res);
    cpu->pc += 1;
    break;
  }
  case sim::isa::Opcode::kHalt: {
    cpu->finished = true;
    break;
  }
  case sim::isa::Opcode::kJump: {
    cpu->pc = cpu->getReg(insn.dst);
    break;
  }
  case sim::isa::Opcode::kLoad: {
    auto data = cpu->memory->load(cpu->getReg(insn.src1));
    cpu->setReg(insn.dst, data);
    cpu->pc += 1;
    break;
  }
  case sim::isa::Opcode::kStore: {
    auto data = cpu->getReg(insn.dst);
    cpu->memory->store(cpu->getReg(insn.src1), data);
    cpu->pc += 1;
    break;
  }
  case sim::isa::Opcode::kBeq: {
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
#include "code.hpp"
  };

  sim::Memory mem;

  // Define entry point
  std::size_t entryAddr = 42;

  // Load program to memory
  for (size_t i = 0; i < std::size(program); ++i) {
    mem.store(entryAddr + i, program[i]);
  }

  // Init cpu state
  sim::CpuState cpu{&mem};

  // set entry point
  cpu.pc = entryAddr;

    // init regfile
#include "regfile.ini"

  // main loop
  while (!cpu.finished) {
    auto bytes = cpu.memory->load(cpu.pc);
    sim::isa::Instruction insn = sim::decoder::decode(bytes);

    execute(&cpu, insn);
  }

  cpu.dump(std::cout);
  std::cout << "Done execution" << std::endl;
}
