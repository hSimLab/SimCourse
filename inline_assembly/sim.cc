#include <cassert>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "capsule_asm.hh"
#include "sim/cpu_state.hh"
#include "sim/decoder.hh"
#include "sim/hart.hh"
#include "sim/isa.hh"
#include "sim/memory.hh"

namespace sim {

class InlineAssemly : public Hart {
    void execute(isa::Instruction insn) override {
        switch (insn.opc) {
            case isa::Opcode::kAdd: {
                ARITHM_CAPSULE("add", cpu.regs[insn.dst], cpu.regs[insn.src1],
                               cpu.regs[insn.src2])

                ADVANCE_PC(cpu.pc)
                break;
            }
            case isa::Opcode::kHalt: {
                cpu.finished = true;
                break;
            }
            case isa::Opcode::kJump: {
                asm volatile("mov %[rd], %[pc]\n\t"
                             : [pc] "+r"(cpu.pc)
                             : [rd] "r"(cpu.regs[insn.dst]));

                break;
            }
            case isa::Opcode::kLoad: {
                LOAD_CAPSULE(cpu.regs[insn.dst],
                             cpu.memory->get_data().at(cpu.regs[insn.src1]))

                ADVANCE_PC(cpu.pc)
                break;
            }
            case isa::Opcode::kStore: {
                STORE_CAPSULE(cpu.regs[insn.dst],
                              cpu.memory->get_data().at(cpu.regs[insn.src1]))

                ADVANCE_PC(cpu.pc)
                break;
            }
            case isa::Opcode::kBeq: {
                B_COND_CAPSULE("je", cpu.pc, cpu.regs[insn.dst],
                               cpu.regs[insn.src1], cpu.regs[insn.src2])
                break;
            }
            default:
                assert(false && "Unknown instruction");
        }
    }
};
}  // namespace sim

int main() {
    // Define program
    std::vector<uint32_t> program = {
#include "code.hpp"
    };

    sim::InlineAssemly model{};
    sim::do_sim(&model, program);

    model.dump(std::cout);
}
