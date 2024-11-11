import random
import argparse

TB_SIZE = 15
REG_FILE_SIZE = 31
K_MAX_VAL = 100000
ENTRY_POINT = 42


def gen_binop(opcode: int , mnemonic : str, rd : int, rs1 : int, rs2 : int):
    return "0x" + f"{opcode:02x}{rd:02x}{rs1:02x}{rs2:02x},\
                // {mnemonic} x{rd}, x{rs1}, x{rs2}"

def gen_store_op(opcode: int, mnemonic : str, rd : int, rs1 : int):
    return "0x" + f"{opcode:02x}{rd:02x}{rs1:02x}00,\
                // {mnemonic} x{rd}, x{rs1}"


def gen_load_op(opcode: int, mnemonic: str, rd: int, rs1: int):
    return "0x" + f"{opcode:02x}{rd:02x}{rs1:02x}00,\
                // {mnemonic} x{rd}, x{rs1}"

def gen_regfile_ini(icount : int, code_section_begin : int, iter_num : int):
    with open(f"regfile.ini", mode='w+') as f:
        print(f"cpu.regs[0] = {ENTRY_POINT};", file=f)
        print(f"cpu.regs[1] = 0; // init value", file=f)
        print(f"cpu.regs[2] = 1; // reduction value", file=f)
        print(f"cpu.regs[3] = {iter_num}; // loop iter num", file=f)
        print(
            f"cpu.regs[30] = {ENTRY_POINT + code_section_begin}; // code start", file=f)
        print(
            f"cpu.regs[31] = {ENTRY_POINT + icount}; // final pc", file=f)

def gen_bytecode() -> int:
    code : list[str] = []
    icount = 0

    code += ["#ifndef SAMPLE_CODE_HH"]
    code += ["#define SAMPLE_CODE_HH"]

    code += [gen_store_op(5, 'store', rd=2, rs1=2)]
    icount += 1

    for id in range(4, REG_FILE_SIZE - 2):
        code += [gen_load_op(4, 'load', rd=id, rs1=2)]
        icount += 1

    code += ["0x031e0000,  // jump x30"]
    icount += 1
    init_section_end = icount

    for id in range(TB_SIZE):
        rd = random.randint(4, REG_FILE_SIZE - 2)
        rs1 = random.randint(4, REG_FILE_SIZE - 2)
        rs2 = random.randint(4, REG_FILE_SIZE - 2)
        code += [gen_binop(1, mnemonic="add", rd=rd, rs1=rs1, rs2=rs2)]
        icount += 1

    code += ["0x01010102,  // add x1, x1, x2"]
    code += ["0x061f0103,  // beq x31, x1, x3"]
    code += ["0x03000000,  // jump x0"]
    icount += 3

    code += ["0x02000000,  // halt"]
    code += ["#endif // SAMPLE_CODE_HH"]

    with open(f"code.hpp", mode='w+') as f:
        for line in code:
            print(line, file=f)

    return icount, init_section_end

def main():
    parser = argparse.ArgumentParser(description='Testgen script')
    parser.add_argument('--iter_num', type=int, default=10000, help='Number of iterations into the test loop')
    args = parser.parse_args()

    icount, init_section_end = gen_bytecode()
    gen_regfile_ini(icount, init_section_end, args.iter_num)

if __name__ == "__main__":
    main()
