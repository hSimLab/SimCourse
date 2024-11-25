#include <cassert>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <vector>

#include "sim/cpu_state.hh"
#include "sim/decoder.hh"
#include "sim/hart.hh"
#include "sim/isa.hh"
#include "sim/memory.hh"

class NaiveInertpreter : public sim::Hart {
    /**
     * @brief Interpeter exectution logic

     * @param[in] insn executee instruction
     */
    void execute(sim::isa::Instruction insn) override {
        step(insn);
    }
};

int main() {
    // Define program
    std::vector<uint32_t> program = {
#include "code.hpp"
    };

    NaiveInertpreter model{};
    sim::do_sim(&model, program);

    model.dump(std::cout);
}
