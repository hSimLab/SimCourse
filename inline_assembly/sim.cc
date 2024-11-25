#include <cassert>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "sim/cpu_state.hh"
#include "sim/decoder.hh"
#include "sim/isa.hh"
#include "sim/memory.hh"

#include "capsule_asm.hh"

void execute(sim::CpuState *cpu, sim::isa::Instruction insn) {
    switch (insn.opc) {
        case sim::isa::Opcode::kAdd: {
            ARITHM_CAPSULE("add", cpu->regs[insn.dst], cpu->regs[insn.src1],
                           cpu->regs[insn.src2])

            ADVANCE_PC(cpu->pc)
            break;
        }
        case sim::isa::Opcode::kHalt: {
            cpu->finished = true;
            break;
        }
        case sim::isa::Opcode::kJump: {
            asm volatile("mov %[rd], %[pc]\n\t"
                         : [pc] "+r"(cpu->pc)
                         : [rd] "r"(cpu->regs[insn.dst]));

            break;
        }
        case sim::isa::Opcode::kLoad: {
            LOAD_CAPSULE(cpu->regs[insn.dst],
                         cpu->memory->get_data().at(cpu->regs[insn.src1]))

            ADVANCE_PC(cpu->pc)
            break;
        }
        case sim::isa::Opcode::kStore: {
            STORE_CAPSULE(cpu->regs[insn.dst],
                          cpu->memory->get_data().at(cpu->regs[insn.src1]))

            ADVANCE_PC(cpu->pc)
            break;
        }
        case sim::isa::Opcode::kBeq: {
            B_COND_CAPSULE("je", cpu->pc, cpu->regs[insn.dst],
                           cpu->regs[insn.src1], cpu->regs[insn.src2])
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
    //

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
