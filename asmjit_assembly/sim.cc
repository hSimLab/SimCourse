#include <asmjit/asmjit.h>

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

class JitAssembly : public sim::Hart {
public:
    using FuncTy = void (*)();

private:
    void execute(sim::isa::Instruction insn) override {
        asmjit::CodeHolder code;
        code.init(runtime.environment());

        asmjit::x86::Assembler assembler{&code};

        switch (insn.opc) {
            case sim::isa::Opcode::kAdd: {
                assembler.mov(asmjit::x86::eax, cpu.regs[insn.src1]);
                assembler.add(asmjit::x86::eax, cpu.regs[insn.src2]);
                //
                assembler.mov(
                    asmjit::x86::dword_ptr((size_t)(&(cpu.regs[insn.dst]))),
                    asmjit::x86::eax);

                assembler.mov(asmjit::x86::eax, cpu.pc);
                assembler.add(asmjit::x86::eax, 1);

                assembler.mov(asmjit::x86::dword_ptr((size_t)(&cpu.pc)),
                              asmjit::x86::eax);

                assembler.ret();
                break;
            }
            case sim::isa::Opcode::kHalt: {
                cpu.finished = true;
                assembler.ret();
                break;
            }
            case sim::isa::Opcode::kJump: {
                assembler.mov(asmjit::x86::eax, cpu.getReg(insn.dst));
                assembler.mov(asmjit::x86::dword_ptr((size_t)(&cpu.pc)),
                              asmjit::x86::eax);

                assembler.ret();
                break;
            }
            case sim::isa::Opcode::kLoad: {
                assembler.mov(asmjit::x86::eax,
                              cpu.memory->load(cpu.regs[insn.src1]));

                assembler.mov(
                    asmjit::x86::dword_ptr((size_t)(&(cpu.regs[insn.dst]))),
                    asmjit::x86::eax);

                assembler.mov(asmjit::x86::eax, cpu.pc);
                assembler.add(asmjit::x86::eax, 1);
                assembler.mov(asmjit::x86::dword_ptr((size_t)(&cpu.pc)),
                              asmjit::x86::eax);
                assembler.ret();
                break;
            }
            case sim::isa::Opcode::kStore: {
                assembler.mov(
                    asmjit::x86::dword_ptr((size_t)(&cpu.memory->get_data().at(
                        cpu.regs[insn.src1]))),
                    cpu.regs[insn.dst]);

                assembler.mov(asmjit::x86::eax, cpu.pc);
                assembler.add(asmjit::x86::eax, 1);

                assembler.mov(asmjit::x86::dword_ptr((size_t)(&cpu.pc)),
                              asmjit::x86::eax);

                assembler.ret();
                break;
            }
            case sim::isa::Opcode::kBeq: {
                asmjit::Label beq_beg = assembler.newLabel(),
                              beq_end = assembler.newLabel();

                assembler.mov(asmjit::x86::eax, cpu.regs[insn.src1]);
                assembler.cmp(asmjit::x86::eax, cpu.regs[insn.src2]);

                assembler.je(beq_beg);

                assembler.mov(asmjit::x86::eax, cpu.pc);
                assembler.add(asmjit::x86::eax, 1);
                assembler.jmp(beq_end);

                assembler.bind(beq_beg);

                assembler.mov(asmjit::x86::eax, cpu.pc);
                assembler.mov(asmjit::x86::eax, cpu.regs[insn.dst]);

                assembler.bind(beq_end);

                assembler.mov(asmjit::x86::dword_ptr((size_t)(&cpu.pc)),
                              asmjit::x86::eax);

                assembler.ret();
                break;
            }
            default:
                assert(false && "Unknown instruction");
        }

        FuncTy exec_func{};
        asmjit::Error err = runtime.add(&exec_func, &code);
        //
        if (err) {
            std::cout << "AsmJit failed: "
                      << asmjit::DebugUtils::errorAsString(err) << std::endl;
            return;
        }

        exec_func();
        runtime.release(exec_func);
    }

    asmjit::JitRuntime runtime;
};

int main() {
    // Define program
    std::vector<uint32_t> program = {
#include "code.hpp"
    };

    JitAssembly model{};
    sim::do_sim(&model, program);

    model.dump(std::cout);
}
