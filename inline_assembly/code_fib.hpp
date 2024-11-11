#ifndef SAMPLE_CODE_HH
#define SAMPLE_CODE_HH

// Инициализация значений
// x[0] = 0
// x[1] = 1
// x[2] = n

0x05030000,    // store x3, x0
0x05040100,    // store x4, x1

// loop_begin
0x04030000,    // load x3, x0
0x04040100,    // load x4, x1
0x01050304,    // add x5, x3, x4
0x07020201,    // sub x2, x2, x1

0x05040000,    // store x4, x0
0x05050100,    // store x5, x1

0x06070201,    // beq end_pc, x2, x0
0x03080000,    // jump loop_begin

// end_pc

0x02000000,    // halt

#endif // SAMPLE_CODE_HH
