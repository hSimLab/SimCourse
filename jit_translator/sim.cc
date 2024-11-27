#include <asmjit/asmjit.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <unordered_map>
#include <vector>

#include "sim/cpu_state.hh"
#include "sim/decoder.hh"
#include "sim/hart.hh"
#include "sim/isa.hh"
#include "sim/memory.hh"

namespace sim {
class JitTranslator : public Hart {
public:
    static constexpr std::size_t KBbThreshold = 10;
    static constexpr std::size_t kAverageBbSize = 10;

    using FuncTy = void (*)();

private:
    isa::Word fetch(CpuState *cpu) {
        return cpu->memory->load(cpu->pc);
    }
    isa::Word fetch(CpuState *cpu, isa::Word addr) {
        return cpu->memory->load(addr);
    }

    // helpers
    static isa::Word loadHelper(Memory *mem, isa::Word addr) {
        return mem->load(addr);
    }
    static void storeHelper(Memory *mem, isa::Word addr,
                            isa::Word value) {
        return mem->store(addr, value);
    }

    bool is_terminate(isa::Opcode opc) {
        switch (opc) {
            case isa::Opcode::kJump:
            case isa::Opcode::kBeq:
            case isa::Opcode::kHalt:
                return true;
                break;
            default:
                return false;
                break;
        }
        return false;
    }

    std::vector<isa::Instruction> lookup_bb(isa::Word addr) {
        auto [find_res, wasNew] = bb_cache.try_emplace(addr);

        if (wasNew) {
            auto cur_addr = addr;
            auto &bb = find_res->second;
            isa::Instruction insn{};
            bb.reserve(kAverageBbSize);
            //
            do {
                auto bytes = fetch(&cpu, cur_addr);
                insn = decoder::decode(bytes);
                bb.push_back(insn);
                cur_addr += 1;

            } while (!is_terminate(insn.opc));
        }

        return find_res->second;
    }

    std::pair<FuncTy, std::size_t> translate(
        const std::vector<isa::Instruction> &bb) {
        std::size_t icount = 0;
        asmjit::CodeHolder code;
        code.init(runtime.environment());
        asmjit::FileLogger logger{stdout};

        asmjit::x86::Compiler cc{&code};
        cc.addFunc(asmjit::FuncSignature::build<void>());

        auto toDwordPtr = [&](auto &arg) {
            return asmjit::x86::dword_ptr((size_t)(&arg));
        };

        auto temp = cc.newGpd();
        auto temp2 = cc.newGpd();
        auto toRet = cc.newGpd();

        for (auto insn : bb) {
            switch (insn.opc) {
                case isa::Opcode::kAdd: {
                    cc.mov(temp, toDwordPtr(cpu.regs[insn.src1]));
                    cc.mov(temp2, toDwordPtr(cpu.regs[insn.src2]));
                    cc.add(temp, temp2);
                    //
                    cc.mov(
                        asmjit::x86::dword_ptr((size_t)(&(cpu.regs[insn.dst]))),
                        temp);

                    cc.mov(temp, toDwordPtr(cpu.pc));
                    cc.add(temp, 1);

                    cc.mov(asmjit::x86::dword_ptr((size_t)(&cpu.pc)), temp);

                    break;
                }
                case isa::Opcode::kHalt: {
                    cc.mov(asmjit::x86::byte_ptr((size_t)&cpu.finished), 1);
                    cc.ret();
                    break;
                }
                case isa::Opcode::kJump: {
                    cc.mov(temp, toDwordPtr(cpu.regs[insn.dst]));
                    cc.mov(asmjit::x86::dword_ptr((size_t)(&cpu.pc)), temp);

                    cc.ret();
                    break;
                }
                case isa::Opcode::kLoad: {
                    asmjit::InvokeNode *invoke{};
                    cc.mov(temp, toDwordPtr(cpu.regs[insn.src1]));

                    cc.invoke(
                        &invoke, (size_t)loadHelper,
                        asmjit::FuncSignature::build<
                            isa::Word, Memory *, isa::Word>());
                    invoke->setArg(0, cpu.memory);
                    invoke->setArg(1, temp);
                    invoke->setRet(0, toRet);

                    cc.mov(toDwordPtr(cpu.regs[insn.dst]), toRet);

                    cc.mov(temp, toDwordPtr(cpu.pc));
                    cc.add(temp, 1);
                    cc.mov(asmjit::x86::dword_ptr((size_t)(&cpu.pc)), temp);
                    break;
                }
                case isa::Opcode::kStore: {
                    cc.mov(temp, toDwordPtr(cpu.regs[insn.dst]));
                    cc.mov(toRet, toDwordPtr(cpu.regs[insn.src1]));

                    asmjit::InvokeNode *invoke{};
                    cc.invoke(&invoke, (size_t)storeHelper,
                              asmjit::FuncSignature::build<void, Memory *,
                                                           isa::Word,
                                                           isa::Word>());
                    invoke->setArg(0, cpu.memory);
                    invoke->setArg(1, toRet);
                    invoke->setArg(2, temp);

                    cc.mov(temp, toDwordPtr(cpu.pc));
                    cc.add(temp, 1);

                    cc.mov(asmjit::x86::dword_ptr((size_t)(&cpu.pc)), temp);

                    break;
                }
                case isa::Opcode::kBeq: {
                    asmjit::Label beq_beg = cc.newLabel(),
                                  beq_end = cc.newLabel();

                    cc.mov(temp, toDwordPtr(cpu.regs[insn.src1]));
                    cc.mov(temp2, toDwordPtr(cpu.regs[insn.src2]));
                    cc.cmp(temp, temp2);

                    cc.je(beq_beg);

                    cc.mov(temp, toDwordPtr(cpu.pc));
                    cc.add(temp, 1);
                    cc.jmp(beq_end);

                    cc.bind(beq_beg);

                    cc.mov(temp, toDwordPtr(cpu.regs[insn.dst]));

                    cc.bind(beq_end);

                    cc.mov(asmjit::x86::dword_ptr((size_t)(&cpu.pc)), temp);

                    cc.ret();
                    break;
                }
                default:
                    assert(false && "Unknown instruction");
            }
        }

        cc.endFunc();
        cc.finalize();

        FuncTy exec_func{};
        asmjit::Error err = runtime.add(&exec_func, &code);
        //
        if (err) {
            std::cout << "AsmJit failed: "
                      << asmjit::DebugUtils::errorAsString(err) << std::endl;

            return {nullptr, icount};
        }

        return {exec_func, icount};
    }

    void interpret(const std::vector<isa::Instruction> &bb) {
        for (std::size_t i = 0; i < bb.size(); ++i) Hart::step();
    }

    void step() override {
        auto find_res = translated.find(cpu.pc);
        if (find_res != translated.end()) {
            find_res->second.first();
            return;
        } else if (bb_cache.count(cpu.pc)) {
            auto &&basic_block = bb_cache[cpu.pc];
            if (basic_block.size() > KBbThreshold) {
                auto [jit_func, icount] = translate(basic_block);

                if (jit_func) {
                    translated[cpu.pc] = std::make_pair(jit_func, icount);
                    jit_func();
                    return;
                }
            }
        }
        auto &&basic_block = lookup_bb(cpu.pc);
        interpret(basic_block);
    }

    asmjit::JitRuntime runtime;
    std::unordered_map<isa::Word, std::vector<isa::Instruction>>
        bb_cache;
    std::unordered_map<isa::Word, std::pair<FuncTy, size_t>> translated;
};
}  // namespace sim

int main() {
    // Define program
    std::vector<uint32_t> program = {
#include "code.hpp"
    };

    sim::JitTranslator model{};
    sim::do_sim(&model, program);

    model.dump(std::cout);
}
