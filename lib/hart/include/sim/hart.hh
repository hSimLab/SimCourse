#ifndef __LIB_HART_INCLUDE_SIM_HART_HH__
#define __LIB_HART_INCLUDE_SIM_HART_HH__

#include <cstddef>
#include <iostream>

#include "sim/cpu_state.hh"
#include "sim/memory.hh"

namespace sim {

struct Hart {
    static constexpr std::size_t kEntryAddr = 42;

    Memory mem{};
    CpuState cpu;

    Hart() : cpu(&mem) {
        // Init cpu state
        cpu.pc = kEntryAddr;
        // init regfile
#include "regfile.ini"
    }

    virtual void execute([[maybe_unused]] sim::isa::Instruction insn) {
        assert(
            false &&
            "Error: function to execute single instruction isn't implemented");
    }

    /**
     * @brief Base interpreter step
     *
     * @param[in] insn incoming instruction
     */
    void step(sim::isa::Instruction insn) {
        switch (insn.opc) {
            case sim::isa::Opcode::kAdd: {
                auto res = cpu.getReg(insn.src1) + cpu.getReg(insn.src2);
                cpu.setReg(insn.dst, res);
                cpu.pc += 1;
                break;
            }
            case sim::isa::Opcode::kHalt: {
                cpu.finished = true;
                break;
            }
            case sim::isa::Opcode::kJump: {
                cpu.pc = cpu.getReg(insn.dst);
                break;
            }
            case sim::isa::Opcode::kLoad: {
                auto data = cpu.memory->load(cpu.getReg(insn.src1));
                cpu.setReg(insn.dst, data);
                cpu.pc += 1;
                break;
            }
            case sim::isa::Opcode::kStore: {
                auto data = cpu.getReg(insn.dst);
                cpu.memory->store(cpu.getReg(insn.src1), data);
                cpu.pc += 1;
                break;
            }
            case sim::isa::Opcode::kBeq: {
                bool eq = cpu.getReg(insn.src1) == cpu.getReg(insn.src2);
                cpu.pc = eq ? cpu.getReg(insn.dst) : cpu.pc + 1;
                break;
            }
            default:
                assert(false && "Unknown instruction");
        }
    }

    /**
     * @brief Function to load program into memory

     * @param[in] program - incoming bytecode of the program
     */
    void load(const std::vector<uint32_t>& program) {
        for (size_t i = 0; i < program.size(); ++i) {
            mem.store(kEntryAddr + i, program[i]);
        }
    }

    /**
     * @brief Main simulation loop
     */
    virtual void run() {
        while (!cpu.finished) {
            auto bytes = cpu.memory->load(cpu.pc);
            sim::isa::Instruction insn = sim::decoder::decode(bytes);
            execute(insn);
        }
    }

    void dump(std::ostream& ost) const {
        cpu.dump(ost);
        ost << "Done execution" << std::endl;
    }
};

void do_sim(Hart* hart, const std::vector<uint32_t>& program) {
    hart->load(program);
    hart->run();
}

}  // namespace sim

#endif  // __LIB_HART_INCLUDE_SIM_HART_HH__
