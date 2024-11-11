#ifndef SAMPLE_CODE_HH
#define SAMPLE_CODE_HH
0x05020200,                // store x2, x2
0x04040200,                // load x4, x2
0x04050200,                // load x5, x2
0x04060200,                // load x6, x2
0x04070200,                // load x7, x2
0x04080200,                // load x8, x2
0x04090200,                // load x9, x2
0x040a0200,                // load x10, x2
0x040b0200,                // load x11, x2
0x040c0200,                // load x12, x2
0x040d0200,                // load x13, x2
0x040e0200,                // load x14, x2
0x040f0200,                // load x15, x2
0x04100200,                // load x16, x2
0x04110200,                // load x17, x2
0x04120200,                // load x18, x2
0x04130200,                // load x19, x2
0x04140200,                // load x20, x2
0x04150200,                // load x21, x2
0x04160200,                // load x22, x2
0x04170200,                // load x23, x2
0x04180200,                // load x24, x2
0x04190200,                // load x25, x2
0x041a0200,                // load x26, x2
0x041b0200,                // load x27, x2
0x041c0200,                // load x28, x2
0x031e0000,  // jump x30
0x011c1b13,                // add x28, x27, x19
0x01181008,                // add x24, x16, x8
0x01040408,                // add x4, x4, x8
0x0107050b,                // add x7, x5, x11
0x01130b1c,                // add x19, x11, x28
0x01161d14,                // add x22, x29, x20
0x010e0e11,                // add x14, x14, x17
0x010f0811,                // add x15, x8, x17
0x011a1b1a,                // add x26, x27, x26
0x0117110b,                // add x23, x17, x11
0x010f1517,                // add x15, x21, x23
0x01160b05,                // add x22, x11, x5
0x01191906,                // add x25, x25, x6
0x01121c13,                // add x18, x28, x19
0x01160619,                // add x22, x6, x25
0x01010102,  // add x1, x1, x2
0x061f0103,  // beq x31, x1, x3
0x03000000,  // jump x0
0x02000000,  // halt
#endif // SAMPLE_CODE_HH
