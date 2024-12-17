#ifndef __LIB_HART_INCLUDE_SIM_HART_HH__
#define __LIB_HART_INCLUDE_SIM_HART_HH__

#include <cstddef>
#include <iostream>

#include "sim/cpu_state.hh"
#include "sim/logger.hh"
#include "sim/memory.hh"
//
#include "timer.hh"

namespace sim {

struct Hart {
    static constexpr std::size_t kEntryAddr = 42;

    Memory mem{};
    CpuState cpu;
    Logger logger;

    std::size_t icount = 0;

    Hart() : cpu(&mem), logger(&cpu) {
        // Init cpu state
        cpu.pc = kEntryAddr;
        // Init regfile
#include "regfile.ini"
    }

    /**
     * @brief Function to load program into memory

     * @param[in] program -- incoming bytecode of the program
    */
    void load(const std::vector<uint32_t>& program) {
        for (size_t i = 0; i < program.size(); ++i) {
            mem.store(kEntryAddr + i, program[i]);
        }
    }
    isa::Word fetch(isa::Word addr) { return mem.load(addr); }

    /**
     * @brief Base interpreter execute stage
     *
     * @param[in] insn -- incoming instruction
     */
    void virtual execute(sim::isa::Instruction insn) {
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
     * @brief Main simulation loop
              Function may be overridden according to implementation specific
     details of model
     */
    virtual void run() {
        while (!cpu.finished) {
            step();
        }
    }

    /**
     * @brief Base interpreter single step
              Function load instruction by current pc from the memory then
                       decode it and pass to execute
     */
    virtual void step() {
        auto cur_pc = cpu.pc;
        auto bytes = fetch(cur_pc);
        sim::isa::Instruction insn = sim::decoder::decode(bytes);
        execute(insn);
        logger.dump(insn, bytes, cur_pc, icount);
        ++icount;
    }

    void set_logger(const std::string& filename) { logger.set(filename); }

    void dump(std::ostream& ost) const {
        cpu.dump(ost);
        ost << "Done execution" << std::endl;
    }
};

void do_sim(Hart* hart, const std::vector<uint32_t>& program) {
    Time::Timer timer{};
    //
    hart->load(program);
    hart->run();
    std::cout << "Time elapsed in microseconds "
              << timer.elapsed<std::chrono::microseconds>() << std::endl;
}

}  // namespace sim

#endif  // __LIB_HART_INCLUDE_SIM_HART_HH__
