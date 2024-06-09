#ifndef EX4_SIM_MEM_H
#define EX4_SIM_MEM_H
#define MEMORY_SIZE 200
extern char main_memory[MEMORY_SIZE];
typedef struct page_descriptor {
    bool valid;
    int frame;
    bool dirty;
    int swap_index;
    int time;
} page_descriptor;

class sim_mem {
    int swapfile_fd; //swap file fd
    int program_fd; //executable file fd
    int text_size;//size of the text part
    int data_size;//size of the data part
    int bss_size;//size of the bss part
    int heap_stack_size;//size of the heap stack part
    int num_of_pages;//the total number of pages
    int page_size;//size of each page
    int num_of_txt_pages;//the number of text pages
    int num_of_data_pages;//the number of data pages
    int num_of_bss_pages;//the number of bss pages
    int num_of_stack_heap_pages;//the number of heap stack pages
    page_descriptor **page_table; //pointer to page table
    bool* frames;

private:
    void decimalToBinary(int decimal, int ad[]);

    void copyAr(int source[], int dest[], int start, int end);

    int binaryToDecimal(int binary[], int size);

    void storeMain(int pageType, int pageNumber, int o);

    int findF();

    void storeSwap(int pageType, int pageNumber, int f);

    int findS();

    void clearSwapFrame(int pageType, int pageNumber, int swap);

    void loadFromSwap();

    int *oldestPage();

public:
    sim_mem(char exe_file_name[], char swap_file_name[], int text_size,
            int data_size, int bss_size, int heap_stack_size,
            int page_size);

    ~sim_mem();

    char load(int address);

    void store(int address, char value);

    void print_memory();

    void print_swap();

    void print_page_table();


#endif //EX4_SIM_MEM_H

    bool legalAddres(int address, int memoryType);
};