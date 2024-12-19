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

namespace sim {
class NaiveInertpreter : public Hart {};
}

int main() {
    // Define program
    std::vector<uint32_t> program = {
#include "code.hpp"
    };

    sim::NaiveInertpreter model{};
    sim::do_sim(model, program);
    model.dump(std::cout);
    std::cout << "Icount = " << model.icount << std::endl;
}
