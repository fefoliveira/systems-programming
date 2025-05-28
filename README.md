# systems-programming

Repository with the project developed during the Systems Programming discipline of the Computer Engineering degree at UERGS.

## 📁 Project Structure

```
.
├── LICENSE             # License of the project (default MIT or custom)
├── Makefile            # Compilation automation using `make`
├── programs
│   └── program.txt     # Example program written using the VM instruction format
├── README.md           # This file
├── src
│   ├── cpu
│   │   ├── cpu.c       # Implementation of instruction execution logic
│   │   └── cpu.h       # Header for cpu.c
│   ├── isa.h           # Instruction Set Architecture: defines opcodes and instruction format
│   ├── loader
│   │   ├── loader.c    # Loads a text-based instruction file into the instruction memory
│   │   └── loader.h    # Header loader.c
│   ├── main.c          # Program entry point: initializes memory, loads program, runs VM
│   └── memory
│       ├── memory.c    # Defines the instruction and data memory arrays and memory-related functions
│       └── memory.h    # Header for memory module (registers, PC, function declarations)
└── vm                  # Binary executable generated after compilation
```

## ⚙️ How to Compile and Execute

Make sure you're in the project root directory. Then run:

```bash
make run program
```

This will:

- Compile all `.c` files
- Execute the virtual machine with the contents of `programs/program.txt`

## 🧠 How it Works

- The VM has a memory of 320 words (270 for instructions and 50 for data) and 4 registers (a0, a1, a2, a3) + PC
- Instructions are manually encoded in `program.txt`
- The loader reads these instructions into `memory[].instr`
- `run_vm()` interprets and executes instructions one by one until `STP` is encountered

## 🛠️ Development Notes

- You can edit `programs/program.txt` to change the program executed
- Each line must follow the opcode format from `isa.h`
- Memory can be preloaded with values by editing `main.c`

## 📄 License

This project is licensed under the terms described in the `LICENSE` file.

---

Made for educational purposes.
