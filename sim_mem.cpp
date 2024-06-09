#include "sim_mem.h"
#include <iostream>
#include <csignal>
#include <fcntl.h>
#include <cstring>
#include <cmath>
#include <sys/stat.h>

//Global variable to count the last time the page used
int timeT = 0;

//constructor to initialize all class parameters
sim_mem::sim_mem(char exe_file_name[], char swap_file_name[], int text_size, int data_size, int bss_size, int heap_stack_size, int page_size) {
    // Initialize all the main memory to '0'
    memset(main_memory, '0', MEMORY_SIZE);
    // Open the exe file
    if ((program_fd = open(exe_file_name, O_RDWR)) == -1) {
        perror("ERR");
        exit(1);
    }
    // Open the swap file
    if ((swapfile_fd = open(swap_file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
        perror("ERR");
        exit(1);
    }

    // Initialize all sizes
    this->text_size = text_size;
    this->data_size = data_size;
    this->bss_size = bss_size;
    this->heap_stack_size = heap_stack_size;
    this->page_size = page_size;
    this->num_of_pages = MEMORY_SIZE / page_size;

    num_of_txt_pages = this->text_size / page_size;
    num_of_data_pages = this->data_size / page_size;
    num_of_bss_pages = this->bss_size / page_size;
    num_of_stack_heap_pages = this->heap_stack_size / page_size;

    frames = new bool[this->num_of_pages];
    for (int i = 0; i < num_of_pages; i++) {
        frames[i] = false;
    }

    // Initialize the page table
    page_table = (page_descriptor **) malloc(sizeof(page_descriptor *) * 4);
    page_table[0] = (page_descriptor *) malloc(sizeof(page_descriptor) * num_of_txt_pages);
    page_table[1] = (page_descriptor *) malloc(sizeof(page_descriptor) * num_of_data_pages);
    page_table[2] = (page_descriptor *) malloc(sizeof(page_descriptor) * num_of_bss_pages);
    page_table[3] = (page_descriptor *) malloc(sizeof(page_descriptor) * num_of_stack_heap_pages);

    for (int i = 0; i < num_of_txt_pages; ++i) {
        page_table[0][i].frame = -1;
        page_table[0][i].swap_index = -1;
        page_table[0][i].valid = false;
        page_table[0][i].dirty = false;
        page_table[0][i].time = INT32_MAX;
    }

    for (int i = 0; i < num_of_data_pages; ++i) {
        page_table[1][i].frame = -1;
        page_table[1][i].swap_index = -1;
        page_table[1][i].valid = false;
        page_table[1][i].dirty = false;
        page_table[1][i].time = INT32_MAX;
    }

    for (int i = 0; i < num_of_bss_pages; ++i) {
        page_table[2][i].frame = -1;
        page_table[2][i].swap_index = -1;
        page_table[2][i].valid = false;
        page_table[2][i].dirty = false;
        page_table[2][i].time = INT32_MAX;
    }

    for (int i = 0; i < num_of_stack_heap_pages; ++i) {
        page_table[3][i].frame = -1;
        page_table[3][i].swap_index = -1;
        page_table[3][i].valid = false;
        page_table[3][i].dirty = false;
        page_table[3][i].time = INT32_MAX;
    }

    // Initialize the swap file to '0'
    char str[(page_size*(num_of_bss_pages+num_of_data_pages+num_of_stack_heap_pages))];
    std::cout<< "page:"<<sizeof(str)<<std::endl;
    memset(str, '0', sizeof(str));
    if (write(swapfile_fd, str, sizeof(str)) == -1)
        perror("1");
}


//function the load from memory
char sim_mem::load(int address) {
    int ad[12];//array to hold the binary address
    decimalToBinary(address, ad);//convert the address to binary
    int mType[2];
    int offsetS = (int) log2(page_size);// calculate how many bits is the offset
    int offset[offsetS];
    int pageNumber[10 - offsetS];
    //copy each part of the address to different array
    copyAr(ad, mType, 0, 2);
    copyAr(ad, pageNumber, 2, 12 - offsetS);
    copyAr(ad, offset, 12 - offsetS, 12);
    //converting each part of the address back to decimal
    int memT = binaryToDecimal(mType, 2);
    if (!legalAddres(address, memT)) {
        printf("ERR\n");
        return '\0';
    }
    int pageN = binaryToDecimal(pageNumber, 10 - offsetS);
    int offs = binaryToDecimal(offset, offsetS);

    //if the requested page is already in the main memory return the requested value
    if (page_table[memT][pageN].valid) {
        int memoryFrame = page_table[memT][pageN].frame;
        timeT++;
        page_table[memT][pageN].time = timeT;
        return main_memory[memoryFrame * page_size + offs];
    } else {
        //if the requested page is not in the main memory
        if (memT == 0) {
            storeMain(memT, pageN, offs);
        } else if (page_table[memT][pageN].dirty) {
            if (findF() == -1) {
                int *ij = oldestPage();
                storeSwap(ij[0], ij[1], page_table[ij[0]][ij[1]].frame * page_size);
                delete[](ij);
            }
            clearSwapFrame(memT, pageN, page_table[memT][pageN].swap_index);

        } else if (memT == 3&&!page_table[memT][pageN].dirty) {
            printf("ERR\n");
            return '\0';
        } else {
            storeMain(memT, pageN, offs);
        }

        int memoryFrame = page_table[memT][pageN].frame;

        timeT++;
        page_table[memT][pageN].time = timeT;
        return main_memory[memoryFrame * page_size + offs];
    }
}



//function to store at the main memory
void sim_mem::store(int address, char value) {
    int ad[12];//array to hold the binary address
    decimalToBinary(address, ad);//convert the address to binary
    int mType[2];
    int offsetS = (int) log2(page_size);// calculate how many bits is the offset
    int offset[offsetS];
    int pageNumber[10 - offsetS];
    //copy each part of the address to different array
    copyAr(ad, mType, 0, 2);
    copyAr(ad, pageNumber, 2, 12 - offsetS);
    copyAr(ad, offset, 12 - offsetS, 12);
    //converting each part of the address back to decimal
    int memT = binaryToDecimal(mType, 2);
    if (!legalAddres(address, memT)) {
        printf("ERR\n");
        return;
    }
    int pageN = binaryToDecimal(pageNumber, 10 - offsetS);
    int offs = binaryToDecimal(offset, offsetS);

    // If the page is in main memory
    if (page_table[memT][pageN].valid) {
        // If the page is a text page, it is read-only
        if (memT == 0) {
            printf("ERR\n");
            return;
        }
        // Write the value to main memory
        main_memory[page_table[memT][pageN].frame * page_size + offs] = value;
        page_table[memT][pageN].dirty = true;
        timeT++;
        page_table[memT][pageN].time = timeT;
    } else {
        // If the page is a text page, it is read-only
        if (memT == 0) {
            printf("ERR\n");
            return;
        } else {
            // If there is no free frame, find the oldest page and move it to swap
            if (findF() == -1) {
                int *ij = oldestPage();
                storeSwap(ij[0], ij[1], page_table[ij[0]][ij[1]].frame * page_size);
                delete[] ij;
            }
            // If the page is dirty, clear the swap frame and load the page back into memory
            if (page_table[memT][pageN].dirty) {
                clearSwapFrame(memT, pageN, page_table[memT][pageN].swap_index);
            } else {
                // Otherwise, load the page from the executable file
                storeMain(memT, pageN, offs);
            }

            int memoryFrame = page_table[memT][pageN].frame;
            if(memT!=1)
            {
                for(int i=memoryFrame * page_size;i<memoryFrame*page_size+page_size;i++)
                {
                    main_memory[i]='0';
                }
            }
            timeT++;
            page_table[memT][pageN].time = timeT;
            page_table[memT][pageN].dirty = true;
            main_memory[memoryFrame * page_size + offs] = value;
        }
    }
}



void sim_mem::print_memory() {
    int i;
    printf("\n Physical memory\n");
    for (i = 0; i < MEMORY_SIZE; i++) {
        printf("[%c]\n", main_memory[i]);
    }
}

void sim_mem::print_swap() {
    char *str = (char *) malloc(this->page_size * sizeof(char));
    int i;
    printf("\n Swap memory\n");
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while (read(swapfile_fd, str, this->page_size) == this->page_size) {
        for (i = 0; i < page_size; i++) {
            printf("%d - [%c]\t", i, str[i]);
        }
        printf("\n");
    }
    free(str);
}

void sim_mem::print_page_table() {
    int i;

    printf("Valid\t Dirty\t Frame\t Swap index\n");
    for ( i = 0; i < num_of_txt_pages ; ++i) {
        printf("[%d]\t [%d]\t [%d]\t [%d]\n",
               page_table[0][i].valid,
               page_table[0][i].dirty,
               page_table[0][i].frame,
               page_table[0][i].swap_index);
    }

    printf("Valid\t Dirty\t Frame\t Swap index\n");
    for ( i = 0; i < num_of_data_pages ; ++i) {
        printf("[%d]\t [%d]\t [%d]\t [%d]\n",
               page_table[1][i].valid,
               page_table[1][i].dirty,
               page_table[1][i].frame,
               page_table[1][i].swap_index);
    }

    printf("Valid\t Dirty\t Frame\t Swap index\n");
    for ( i = 0; i < num_of_bss_pages; ++i) {
        printf("[%d]\t [%d]\t [%d]\t [%d]\n",
               page_table[2][i].valid,
               page_table[2][i].dirty,
               page_table[2][i].frame,
               page_table[2][i].swap_index);
    }

    printf("Valid\t Dirty\t Frame\t Swap index\n");
    for ( i = 0; i < num_of_stack_heap_pages; ++i) {
        printf("[%d]\t [%d]\t [%d]\t [%d]\n",
               page_table[3][i].valid,
               page_table[3][i].dirty,
               page_table[3][i].frame,
               page_table[3][i].swap_index);
    }
}

//function to convert decimal to binary
void sim_mem::decimalToBinary(int decimal, int ad[]) {
    int index = 12;
    while (index > 0) {
        ad[index - 1] = decimal % 2;
        decimal = decimal / 2;
        index--;
    }
}

sim_mem::~sim_mem() {
    close(program_fd);
    close(swapfile_fd);
    delete[](frames);
    for (int i = 0; i < 4; ++i) {
        free(page_table[i]);

    }
    free(page_table);


}

//function to copy part of array
void sim_mem::copyAr(int source[], int dest[], int start, int end) {
    for (int i = 0; i < end; ++i) {
        dest[i] = source[start + i];
    }
}

//function to convert from binary to decimal
int sim_mem::binaryToDecimal(int binary[], int size) {
    int decimal = 0;
    int base = 1;

    for (int i = size - 1; i >= 0; i--) {
        if (binary[i] == 1) {
            decimal += base;
        }
        base *= 2;
    }

    return decimal;
}

//function to find free frame at the main memory
int sim_mem::findF() {
    for (int i = 0; i < this->num_of_pages; i++) {
        if (!frames[i])
            return i * page_size;
    }
    return -1;
}

//function to move page from the disk to the main memory
void sim_mem::storeMain(int pageType, int pageNumber, int o) {
    int type = pageType == 0 ? 0 : pageType == 1 ? text_size : text_size + data_size;
    std::cout<<type<<std::endl;
    int idx = type + (pageNumber * page_size);
    char temp[page_size];
    //reading requested page from exe file
    lseek(program_fd, idx, SEEK_SET);
    read(program_fd, temp, page_size);
    if (findF() == -1) {
        int *ij = oldestPage();
        //move the oldest frame to swap
        storeSwap(ij[0], ij[1], page_table[ij[0]][ij[1]].frame * page_size);
        delete[](ij);
    }
    //find index of new frame at main memory
    int frame_index = findF();
    for (int i = frame_index; i < page_size + frame_index; i++) {
        main_memory[i] = temp[i - frame_index];
    }
    page_table[pageType][pageNumber].valid = true;
    page_table[pageType][pageNumber].frame = frame_index / page_size;
    frames[page_table[pageType][pageNumber].frame] = true;


}

//function to find the next free spot at the swap
int sim_mem::findS() {
    struct stat fileStat;
    if (fstat(swapfile_fd, &fileStat) == -1) {
        printf("Failed to get the file status.\n");
        close(swapfile_fd);
        return -1;
    }
    off_t fileSize = fileStat.st_size;
    char c[1];
    for (int i = 0; i < fileSize; i += page_size) {
        lseek(swapfile_fd, i, SEEK_SET);
        read(swapfile_fd, c, 1);
        if (c[0] == '0')
            return i;
    }
    return -1;
}


//moving frame from the main memory to the swap
void sim_mem::storeSwap(int pageType, int pageNumber, int f) {
    // Text pages should not be moved to swap
    if (pageType == 0 || !(page_table[pageType][pageNumber].dirty)) {
        frames[page_table[pageType][pageNumber].frame] = false;
        page_table[pageType][pageNumber].valid = false;
        page_table[pageType][pageNumber].frame = -1;
        page_table[pageType][pageNumber].time = INT32_MAX;
        return;
    }

    printf("Storing page %d of type %d to swap.\n", pageNumber, pageType);

    int swap_index = findS();
    if (swap_index == -1) {
        printf("Error: No free swap space found.\n");
        return;
    }

    // Store the swap index in the page table
    page_table[pageType][pageNumber].swap_index = swap_index / page_size;

    // Create a temporary buffer to hold the page data
    char temp[page_size];
    for (int i = 0; i < page_size; ++i) {
        temp[i] = main_memory[f + i];
        main_memory[f + i] = '0';  // Clear the main memory frame
    }

    printf("Writing to swap at index %d: ", swap_index);
    for (int i = 0; i < page_size; ++i) {
        printf("%c", temp[i]);
    }
    printf("\n");

    // Write the page data to the swap file
    lseek(swapfile_fd, swap_index, SEEK_SET);
    write(swapfile_fd, temp, page_size);

    // Update the frame status and page table
    frames[page_table[pageType][pageNumber].frame] = false;
    page_table[pageType][pageNumber].valid = false;
    page_table[pageType][pageNumber].frame = -1;
    page_table[pageType][pageNumber].time = INT32_MAX;
    printf("Stored data/bss/stack page %d to swap at index %d.\n", pageNumber, swap_index);
}








//function to check which page at the main memory is the
int *sim_mem::oldestPage() {
    int t = INT32_MAX;
    int *idx = new int[2];
    idx[0] = 0;
    idx[1] = 0;
    int size = 0;
    for (int i = 0; i < 4; ++i) {
        if (i == 0) size = num_of_txt_pages;
        else if (i == 1)size = num_of_data_pages;
        else if (i == 2)size = num_of_bss_pages;
        else size = num_of_stack_heap_pages;
        for (int j = 0; j < size; ++j) {
            if ((page_table[i][j].time < t) && (t > -1)) {
                idx[0] = i;
                idx[1] = j;
                t = page_table[i][j].time;
            }

        }


    }

    return idx;
}

void sim_mem::clearSwapFrame(int pageType, int pageNumber, int swap) {
    printf("Clearing swap frame %d for page %d of type %d.\n", swap, pageNumber, pageType);

    char temp[page_size];
    char zeros[page_size];
    memset(zeros, '0', page_size);

    int frame = findF();
    if (frame == -1) {

        printf("Error: No free frame found in main memory.\n");
        return;
    }

    if (pageType == 0) {
        // Read text page from the executable file
        int type = pageType ;
        int idx = type + pageNumber * page_size;
        lseek(program_fd, idx, SEEK_SET);
        read(program_fd, temp, page_size);

        printf("Reading from executable file at index %d: ", idx);
        for (int i = 0; i < page_size; i++) {
            printf("%c", temp[i]);
        }
        printf("\n");
    } else {
        // Read from swap
        int swap_index = swap;
        lseek(swapfile_fd, swap_index*page_size, SEEK_SET);
        read(swapfile_fd, temp, page_size);

        printf("Reading from swap at index %d: ", swap_index);
        for (int i = 0; i < page_size; ++i) {
            printf("%c", temp[i]);
        }
        printf("\n");

        // Clear the swap space
        lseek(swapfile_fd, swap_index, SEEK_SET);
        write(swapfile_fd, zeros, page_size);
    }

    for (int i = 0; i < page_size; i++) {
        main_memory[frame + i] = temp[i];
    }

    page_table[pageType][pageNumber].valid = true;
    page_table[pageType][pageNumber].frame = frame / page_size;
    page_table[pageType][pageNumber].swap_index = -1;
    frames[frame / page_size] = true;

    printf("Loaded page %d of type %d from swap index %d to frame %d.\n", pageNumber, pageType, swap, frame / page_size);
}







bool sim_mem::legalAddres(int address, int memoryType) {
    if (memoryType == 0 && (address >= 0 && address <= text_size)) {
        return true;
    } else if (memoryType == 1 && (address >= 1024 && address <= 1024 + data_size)) {
        return true;
    } else if (memoryType == 2 && (address >= 2048 && address <= 2048 + bss_size)) {
        return true;
    } else if (memoryType == 3 && (address >= 3072 && address < 3072 + heap_stack_size)) {
        return true;
    }

    return false;
}
