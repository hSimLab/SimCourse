#include <asmjit/asmjit.h>

#include <cassert>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <vector>

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

void execute(CpuState *cpu, Instruction insn) {
    asmjit::CodeHolder code;
    code.init(cpu->runtime.environment());

    asmjit::x86::Assembler assembler{&code};

    switch (insn.opc) {
        case Opcode::kAdd: {
            assembler.mov(asmjit::x86::eax, cpu->regs[insn.src1]);
            assembler.add(asmjit::x86::eax, cpu->regs[insn.src2]);
            //
            assembler.mov(
                asmjit::x86::dword_ptr((size_t)(&(cpu->regs[insn.dst]))),
                asmjit::x86::eax);

            assembler.mov(asmjit::x86::eax, cpu->pc);
            assembler.add(asmjit::x86::eax, 1);

            assembler.mov(asmjit::x86::dword_ptr((size_t)(&cpu->pc)),
                          asmjit::x86::eax);

            assembler.ret();
            break;
        }
        case Opcode::kHalt: {
            cpu->finished = true;
            assembler.ret();
            break;
        }
        case Opcode::kJump: {
            assembler.mov(asmjit::x86::eax, cpu->getReg(insn.dst));
            assembler.mov(asmjit::x86::dword_ptr((size_t)(&cpu->pc)),
                          asmjit::x86::eax);

            assembler.ret();
            break;
        }
        case Opcode::kLoad: {
            assembler.mov(asmjit::x86::eax,
                          cpu->memory->data().at(cpu->regs[insn.src1]));

            assembler.mov(
                asmjit::x86::dword_ptr((size_t)(&(cpu->regs[insn.dst]))),
                asmjit::x86::eax);

            assembler.mov(asmjit::x86::eax, cpu->pc);
            assembler.add(asmjit::x86::eax, 1);
            assembler.mov(asmjit::x86::dword_ptr((size_t)(&cpu->pc)),
                          asmjit::x86::eax);
            assembler.ret();
            break;
        }
        case Opcode::kStore: {
            assembler.mov(asmjit::x86::dword_ptr((size_t)(&cpu->memory->data().at(
                              cpu->regs[insn.src1]))),
                          cpu->regs[insn.dst]);

            assembler.mov(asmjit::x86::eax, cpu->pc);
            assembler.add(asmjit::x86::eax, 1);

            assembler.mov(asmjit::x86::dword_ptr((size_t)(&cpu->pc)),
                          asmjit::x86::eax);

            assembler.ret();
            break;
        }
        case Opcode::kBeq: {
            asmjit::Label beq_beg = assembler.newLabel(),
                          beq_end = assembler.newLabel();

            assembler.mov(asmjit::x86::eax, cpu->regs[insn.src1]);
            assembler.cmp(asmjit::x86::eax, cpu->regs[insn.src2]);

            assembler.je(beq_beg);

            assembler.mov(asmjit::x86::eax, cpu->pc);
            assembler.add(asmjit::x86::eax, 1);
            assembler.jmp(beq_end);

            assembler.bind(beq_beg);

            assembler.mov(asmjit::x86::eax, cpu->pc);
            assembler.mov(asmjit::x86::eax, cpu->regs[insn.dst]);

            assembler.bind(beq_end);

            assembler.mov(asmjit::x86::dword_ptr((size_t)(&cpu->pc)),
                          asmjit::x86::eax);

            assembler.ret();
            break;
        }
        default:
            assert(false && "Unknown instruction");
    }

    CpuState::FuncTy exec_func{};
    asmjit::Error err = cpu->runtime.add(&exec_func, &code);
    //
    if (err) {
        std::cout << "AsmJit failed: " << asmjit::DebugUtils::errorAsString(err)
                  << std::endl;
        return;
    }

    exec_func();
    cpu->runtime.release(exec_func);
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
        auto bytes = fetch(&cpu);
        Instruction insn = decode(bytes);

        execute(&cpu, insn);
    }

    cpu.dump();
    std::cout << "Done execution" << std::endl;
}
