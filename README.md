# systems-programming

Repository with the project developed during the Systems Programming discipline of the Computer Engineering degree at UERGS.

## 📁 Project Structure

```
.
├── .vscode
├── programs
│   ├── program.txt     # Program written using the VM instruction format with mnemonics
│   └── program.obj     # Object file assembled by the two-pass-assembler (this guy gonna run in VM)
├── src
│   ├── assembler
│   │   ├── assembler.c     # Implementation of the two-pass-assembler (assembler utils functions and main())
│   │   ├── assembler.h     # Header for "assembler.c", "first_pass.c" and "second_pass.c"
│   │   ├── single-module
│   │   │   ├── fist_pass.c     # First-pass function for the two-pass-assembler (single-module assembling)
│   │   │   └── second_pass.c   # Second-pass function for the two-pass-assembler (single-module assembling)
│   │   └── multi-module
│   │       ├── linkter_fist_pass.c   # First-pass function for the linker (multi-module assembling)
│   │       └── linker_second_pass.c  # Second-pass function for the linker (multi-module assembling)
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
├── .gitignore
├── .clang-format
├── LICENSE         # License of the project (default MIT or custom)
├── Makefile        # Compilation automation using `make`
├── assembler_bin   # Binary executable generated after assembler compilation
├── vm_bin          # Binary executable generated after vm compilation
└── README.md       # This file
```

## ⚙️ How to Compile and Execute

Make sure you're in the project root directory. Then, <strong>to assemble a single source code</strong>, run:

```bash
make assemble program
```

This will:

- Assemble the source code in "program.txt" into a object file `program.obj` (always with the name of the .`txt` file), which will be be created in the same directory `/programs`.

Once the object file is assembled, run:

```bash
make run program
```

This will:

- Execute the virtual machine with the contents of `programs/program.obj`

Or, <strong>to link two or more source code files and assemble them</strong>, run:

```bash
make link program1 program2 program3
```

This will:

- Link the modules `program1.txt`, `program2.txt` and `program3.txt`, for example, and assemble them into a `linked.obj` (always with this name), which is an object file where all the source codes are linked with the labels resolved. It's possible to link from 2 to 16 files.

Once the object file is assembled, run:

```bash
make run linked
```

This will:

- Execute the virtual machine with the contents of `programs/linked.obj`

## 🧠 How it Works

- The VM has a memory of 320 words (270 for instructions and 50 for data) and 4 registers (a0, a1, a2, a3) + PC
- Instructions are manually encoded in `program.txt` with the mnemonics
- The assembler read this source code, transform it into machine language and put it into a object file `program.obj`. If more than one files is trying to be linked and assembled, the linker is going to resolve all the labels between the source codes and assemble them into a linked object file `linked.obj`, with the respective machine language words
- The loader reads these instructions into `memory[].instr`
- `run_vm()` interprets and executes instructions one by one until `STP` is encountered

## 🛠️ Development Notes

- You can edit `programs/program.txt` (or any other .txt file related to a program, such as an "add.txt") to change the program executed
- Each line must follow the opcode format from `isa.h`
- Memory can be preloaded with values by editing `main.c`
- The "linker" is just another assembler like the sigle-module one, but with the ability to resolve an external symbol table for all the code's labels, and join more than one assembly into one, to later assemble everything together at once

## 📄 License

This project is licensed under the terms described in the `LICENSE` file.

---

Made for educational purposes.
