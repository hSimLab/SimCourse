# CPU & OS simulation Elective Course

## What is the course about?

Сourse repository is dedicated to CPU and OS simulation in third bachelor semester at MIPT.

All teaching materials used during the semester are [here](slides/).

## Demo Code

You can find [simulator library](lib/) and [test generation script](test/) here.

During the course we consistently improve our toy model of the simulator.

1. [Naive Interpreter](naive_interpreter/sim.cc)
2. [Inline Assembly model](inline_assembly/sim.cc)
3. [AsmJit Assembly model](asmjit_assembly/sim.cc)
4. [JIT translator](jit_translator/sim.cc)

## Slides

1. [Introduction](slides/Introduction.pdf)
2. [Software Modeling](slides/Lecture_1_Software_Modeling.pdf)
3. [Interpreters](slides/Lecture_2_Interpreters.pdf)
4. [Decoder](slides/Lecture_3_Decoder.pdf)
5. [ELF](slides/Lecture_4_ELF.pdf)
6. [Advanced Interptreters](slides/Lecture_5_Interpreter+.pdf)
7. [Full-System Simulation](slides/Lecture_6_FSS.pdf)
8. [Trace Driver Simulation](slides/Lecture_7_TDS.pdf)
9. [Cycle-Accurate Models](slides/Lecture_8_CA_models.pdf)
10. [Caches](slides/Lecture_9_Caches.pdf)
11. [Program Execution Analysis](slides/Lecture_10_Program_Execution_Analysis.pdf)

## Usage

From the root of source directory configure:

```bash
mkdir -p build/
cmake -B build -S .
```

Then run build:

```bash
cmake --build build/ -j<nproc>
```
