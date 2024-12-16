#ifndef __LIB_ISA_INCLUDE_SIM_TRACER_HH__
#define __LIB_ISA_INCLUDE_SIM_TRACER_HH__

#include <cstdint>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>

#include "sim/memory.hh"

namespace sim {

class Logger {
public:
    Logger(const CpuState* state) : m_state(state) {}

    void dump(sim::isa::Instruction insn, isa::Word bytes, std::size_t pc = 0,
              std::size_t icount = 0) {
        if (m_enable && m_output.is_open()) {
            bool is_load = insn.opc == isa::Opcode::kLoad,
                 is_store = insn.opc == isa::Opcode::kStore;
            auto dst_value = m_state->getReg(insn.dst);

            m_output << icount << ": " << pc << " ";
            m_output << "0x" << std::hex << bytes << " " << std::dec;

            m_output << std::endl;

            if (is_store) {
                auto src_value = m_state->getReg(insn.src1);

                m_output << "X" << insn.src1 << " ==> 0x" << std::hex
                         << src_value << std::dec << std::endl;
                m_output << "[0x" << std::hex << dst_value << "] <== 0x"
                         << src_value << std::endl;
            } else if (is_load) {
                auto src_value = m_state->getReg(insn.dst);
                auto mem_value = m_state->memory->get_data().at(src_value);

                m_output << "[0x" << std::hex << src_value << "]"
                         << " ==> 0x" << mem_value << std::dec << std::endl;
                m_output << "X" << insn.dst << " <== 0x" << std::hex
                         << mem_value << std::dec << std::endl;
            } else {
                m_output << "X" << insn.dst << " <== " << std::hex
                         << m_state->getReg(insn.dst) << std::dec << std::endl;
            }
        }
    }

    void set(const std::string& filename) {
        m_enable = true;
        m_output.open(filename);
    }

private:
    const CpuState* m_state{nullptr};
    std::ofstream m_output;
    bool m_enable{true};
};
}  // namespace sim

#endif  //! __LIB_ISA_INCLUDE_SIM_ISA_HH__
