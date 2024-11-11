#pragma once

#define ARITHM_CAPSULE(op, dst, src1, src2)         \
    asm volatile("mov %[rs1], %%eax\n\t" op         \
                 " %[rs2], %%eax\n\t"               \
                 "mov %%eax, %[rd]\n\t"             \
                 : [rd] "=r"(dst)                   \
                 : [rs1] "r"(src1), [rs2] "r"(src2) \
                 : "%eax");

#define LOAD_CAPSULE(dst, mem_data) \
    asm volatile("mov %[data], %[rd]" : [rd] "=r"(dst) : [data] "m"(mem_data));

#define STORE_CAPSULE(dst, mem_addr)        \
    asm volatile("mov %[data], %[addr]\n\t" \
                 : [addr] "=m"(mem_addr)    \
                 : [data] "r"(dst));

#define B_COND_CAPSULE(jump_cond, cur_pc, dst, src1, src2) \
    asm volatile(                                          \
        "movl %[rs1], %%eax\n\t"                           \
        "cmpl %[rs2], %%eax\n\t" jump_cond                 \
        " equal\n\t"                                       \
        "incl %[pc]\n\t"                                   \
        "jmp not_equal\n\t"                                \
        "equal:\n\t"                                       \
        "movl %[rd], %[pc]\n\t"                            \
        "not_equal:"                                       \
        "nop\n\t"                                          \
        : [pc] "+r"(cur_pc)                                \
        : [rs1] "r"(src1), [rs2] "r"(src2), [rd] "r"(dst)  \
        : "%eax");

#define ADVANCE_PC(incr) \
    asm volatile("incl %0\n\t" : "=m"(incr) : "m"(incr) : "memory");
