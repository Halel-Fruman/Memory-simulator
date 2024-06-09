
# Simple Memory Management Simulation

This project is a simple simulation of memory management, including handling of main memory, swap space, and various types of memory pages (text, data, BSS, heap, stack). The program provides functionality to load and store data in memory while managing page swaps between main memory and swap space.

## Files

- **main.cpp**: The main file containing the entry point and usage of the memory simulation.
- **sim_mem.cpp**: Implementation of the `sim_mem` class which handles the memory management operations.
- **sim_mem.h**: Header file for the `sim_mem` class.
- **makefile**: The makefile to compile the project.

## Class `sim_mem`

The `sim_mem` class is responsible for simulating memory management. It includes methods for loading and storing data, managing swap space, and maintaining the state of memory pages.

### Methods

- **sim_mem()**: Constructor to initialize memory parameters and open files.
- **~sim_mem()**: Destructor to clean up resources.
- **load(int address)**: Load data from a given address.
- **store(int address, char value)**: Store data at a given address.
- **print_memory()**: Print the current state of main memory.
- **print_swap()**: Print the current state of swap space.
- **print_page_table()**: Print the current state of the page table.

## How to Compile

To compile the project, use the provided `makefile`. Simply run the following command in the terminal:

```sh
make
```

This will compile the project and produce an executable named `Memory_Simulator`.

## How to Run

After compiling, run the executable with the following command:

```sh
./Memory_Simulator
```

This will execute the main program which demonstrates the memory management simulation.

```cpp
int main() {
    sim_mem s((char*)"exec_file",(char*)"swap_file",128,128,64,64,64);
    s.store(1024,'*');
    s.store(1088,'!');
    s.load(0);
    s.load(64);
    s.load(2048);
    s.store(3072,'%');
    s.store(1025,'%');
    s.print_swap();
    s.print_page_table();
    s.print_memory();
    return 0;
}
```





