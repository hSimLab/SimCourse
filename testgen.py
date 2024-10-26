import random

TB_SIZE = 100
REG_FILE_SIZE = 31
K_MAX_VAL = 100000
ENTRY_POINT = 42

DIRS = ["lecture_02", "lecture_05"]


def gen_binop(opcode: int , mnemonic : str, rd : int, rs1 : int, rs2 : int):
    return "0x" + f"{opcode:02x}{rd:02x}{rs1:02x}{rs2:02x},\
                // {mnemonic} x{rd}, x{rs1}, x{rs2}"

def gen_store_op(opcode: int, mnemonic : str, rd : int, rs1 : int):
    return "0x" + f"{opcode:02x}{rd:02x}{rs1:02x}00,\
                // {mnemonic} x{rd}, x{rs1}"


def gen_load_op(opcode: int, mnemonic: str, rd: int, rs1: int):
    return "0x" + f"{opcode:02x}{rd:02x}{rs1:02x}00,\
                // {mnemonic} x{rd}, x{rs1}"

def main():

    for directory in DIRS:
        icount = 0
        with open(f"{directory}/code.hpp", mode='w+') as f:
            print("#ifndef SAMPLE_CODE_HH", file=f)
            print("#define SAMPLE_CODE_HH", file=f)

            print(gen_store_op(5, 'store', rd=2, rs1=2), file=f)
            icount += 1

            for id in range(4, REG_FILE_SIZE - 1):
                print(gen_load_op(4, 'load', rd=id, rs1=2), file = f)
                icount += 1

            for id in range(TB_SIZE):
                rd = random.randint(4, REG_FILE_SIZE - 1)
                rs1 = random.randint(4, REG_FILE_SIZE - 1)
                rs2 = random.randint(4, REG_FILE_SIZE - 1)
                print(gen_binop(1, mnemonic="add", rd=rd, rs1=rs1, rs2=rs2), file=f)
                icount += 1

            print("0x01010102,  // add x1, x1, x2", file=f)
            print("0x061f0103,  // beq x31, x1, x3", file=f)
            print("0x03000000,  // jump x0", file=f)
            icount += 3

            print("0x02000000,  // halt", file=f)

            print("#endif // SAMPLE_CODE_HH", file=f)

    for directory in DIRS:
        with open(f"{directory}/regfile.ini", mode='w+') as f:
            print(f"cpu.regs[0] = {ENTRY_POINT};", file=f)
            print(f"cpu.regs[1] = 0; // init value", file=f)
            print(f"cpu.regs[2] = 1; // reduction value", file=f)
            print(f"cpu.regs[3] = 10000000; // loop iter num", file=f)
            print(f"cpu.regs[31] = {ENTRY_POINT + icount}; // final pc", file=f)

if __name__ == "__main__":
    main()
