#include <cassert>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "capsule_asm.hh"

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
    kSub,
};

struct Memory {
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
        case Opcode::kSub:
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
            ARITHM_CAPSULE("add", cpu->regs[insn.dst], cpu->regs[insn.src1],
                           cpu->regs[insn.src2])

            ADVANCE_PC(cpu->pc)
            break;
        }
        case Opcode::kSub: {
            ARITHM_CAPSULE("sub", cpu->regs[insn.dst], cpu->regs[insn.src1],
                           cpu->regs[insn.src2])

            ADVANCE_PC(cpu->pc)
            break;
        }
        case Opcode::kHalt: {
            cpu->finished = true;
            break;
        }
        case Opcode::kJump: {
            asm volatile("mov %[rd], %[pc]\n\t"
                         : [pc] "+r"(cpu->pc)
                         : [rd] "r"(cpu->regs[insn.dst]));

            break;
        }
        case Opcode::kLoad: {
            LOAD_CAPSULE(cpu->regs[insn.dst],
                         cpu->memory->data().at(cpu->regs[insn.src1]))

            ADVANCE_PC(cpu->pc)
            break;
        }
        case Opcode::kStore: {
            STORE_CAPSULE(cpu->regs[insn.dst],
                          cpu->memory->data().at(cpu->regs[insn.src1]))

            ADVANCE_PC(cpu->pc)
            break;
        }
        case Opcode::kBeq: {
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
        0x01010203,  // Add x1, x2, x3
        0x01040203,  // Add x4, x2, x3
        0x07080102,  // Sub x8, x1, x2
        0x06070105,  // Beq x7, x1, x4
        0x03000000,  // Jump x0
        0x01030303,  // Add x3, x3, x3
        0x01020202,  // Add x2, x2, x2
        0x05070200,  // Store x7, x2
        0x04090200,  // Load x9, x2
        0x02000000,  // Halt
    };
    //

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
    cpu.regs[0] = 47;
    cpu.regs[2] = 10;
    cpu.regs[3] = 20;
    cpu.regs[7] = 48;

    // main loop
    while (!cpu.finished) {
        auto bytes = fetch(&cpu);
        Instruction insn = decode(bytes);

        execute(&cpu, insn);
    }

    std::cout << "Done execution" << std::endl;
    cpu.dump();
}
