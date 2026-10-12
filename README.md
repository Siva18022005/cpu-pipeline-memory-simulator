# CPU Pipeline & Memory Subsystem Simulator (C++)

A cycle-accurate 5-stage pipelined processor simulator implemented in C++.

This project models instruction-level execution, hazard detection, forwarding, and memory interactions at the clock-cycle level.

---

## Architecture Overview

### Pipeline Stages
- IF – Instruction Fetch
- ID – Instruction Decode
- EX – Execute
- MEM – Memory Access
- WB – Write Back

Each clock cycle updates:
- Pipeline registers (IF/ID, ID/EX, EX/MEM, MEM/WB)
- Register file
- Memory state
- Hazard detection logic

---

## Features

- 5-stage pipeline simulation
- RAW hazard detection
- Data forwarding
- Stall insertion
- Control hazard handling (branch resolution)
- Custom assembler for binary instruction parsing
- Performance benchmarking (CPI calculation)

---

## Example Result

For a 10-instruction program:

- Total Cycles: 19
- Ideal Cycles (No Hazards): 14
- CPI ≈ 1.9

RAW hazards were detected and resolved using dynamic stalls and forwarding.

---

## Project Structure

cpu-pipeline-simulator/
│
├── src/
│ ├── PipelinedCPU.cpp
│ ├── NonPipelinedCPU.cpp
│ ├── Assembler.cpp
│
├── examples/
│ └── sample_input.txt
│
├── output/
│ └── sample_output.txt
│
└── README.md

---

## How to Run

g++ src/*.cpp -o cpu_sim
./cpu_sim

## Design Philosophy

Built using modular, object-oriented design principles to allow easy extension of ISA instructions, cache policies, and performance experimentation.
