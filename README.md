# systems-programming

Repository with the project developed during the Systems Programming discipline of the Computer Engineering degree at UERGS.

## 📁 Project Structure

```
.
├── LICENSE             # License of the project (default MIT or custom)
├── Makefile            # Compilation automation using `make`
├── programs
│   ├── program.txt     # Program written using the VM instruction format with mnemonics
│   └── program.obj     # Object file assembled by the two-pass-assembler (this guy gonna run in VM)
├── README.md           # This file
├── src
│   ├── assembler
│   │   ├── assembler.c     # Implementation of the two-pass-assembler (assembler utils functions and main())
│   │   ├── assembler.h     # Header for "assembler.c", "first_pass.c" and "second_pass.c"
│   │   ├── fist_pass.c     # First-pass function for the two-pass-assembler
│   │   └── second_pass.c   # Second-pass function for the two-pass-assembler
│   ├── utils
│   │   ├── utils.c       # Implementation of utils functions (like trim())
│   │   └── utils.h       # Header for utils.c
│   └── vm
│       ├── cpu
│       │   ├──
│       │   ├── cpu.c       # Implementation of instruction execution logic
│       │   └── cpu.h       # Header for cpu.c
│       ├── loader
│       │   ├── loader.c    # Loads a text-based instruction file into the instruction memory
│       │   └── loader.h    # Header loader.c
│       ├── memory
│       │   ├── memory.c    # Defines the instruction and data memory arrays and memory-related functions
│       │   └── memory.h    # Header for memory module (registers, PC, function declarations)
│       ├── isa.h           # Instruction Set Architecture: defines opcodes and instruction format
│       └── main.c          # Program entry point: initializes memory, loads program, runs VM
├── assembler_bin   # Binary executable generated after assembler compilation
└── vm_bin          # Binary executable generated after vm compilation
```

## ⚙️ How to Compile and Execute

Make sure you're in the project root directory. Then run:

```bash
make assemble program
```

This will:

- Assemble the source code in "program.txt" into a object file "program.obj", which will be be created in the same directory.
  Once the object file is assembled, run:

```bash
make run program
```

This will:

- Execute the virtual machine with the contents of `programs/program.obj`

## 🧠 How it Works

- The VM has a memory of 320 words (270 for instructions and 50 for data) and 4 registers (a0, a1, a2, a3) + PC
- Instructions are manually encoded in `program.txt` with the mnemonics
- The assembler read this source code, transform it into machine language and put it into a object file `program.obj`
- The loader reads these instructions into `memory[].instr`
- `run_vm()` interprets and executes instructions one by one until `STP` is encountered

## 🛠️ Development Notes

- You can edit `programs/program.txt` (or any other .txt file related to a program, such as an "add.txt") to change the program executed
- Each line must follow the opcode format from `isa.h`
- Memory can be preloaded with values by editing `main.c`

## 📄 License

This project is licensed under the terms described in the `LICENSE` file.

---

Made for educational purposes.
