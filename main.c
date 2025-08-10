#include <stdio.h>

#define MEMORY_SIZE 102400  // 100KB = 100 * 1024 bytes

// Memory block structure for managing allocated and free blocks
typedef struct Block {
    int size;          // Size of the block (in bytes)
    int free;          // 1 if free, 0 if allocated
    struct Block* next;
} Block;

static unsigned char memory[MEMORY_SIZE]; // Raw memory buffer
static Block* freeList = NULL;             // Linked list of memory blocks (initially entire memory free)

static void initialize() {
    // Initialize free list with one big free block
    freeList = (Block*)memory;
    freeList->size = MEMORY_SIZE - sizeof(Block);
    freeList->free = 1;
    freeList->next = NULL;
}

// Split a block if it's bigger than requested size
static void splitBlock(Block* fittingBlock, int size) {
    Block* newBlock = (Block*)((unsigned char*)fittingBlock + sizeof(Block) + size);
    newBlock->size = fittingBlock->size - size - sizeof(Block);
    newBlock->free = 1;
    newBlock->next = fittingBlock->next;

    fittingBlock->size = size;
    fittingBlock->free = 0;
    fittingBlock->next = newBlock;
}

// Allocate memory using first fit strategy
int* allocate(int size) {
    if (size <= 0 || size > MEMORY_SIZE) return NULL;

    if (!freeList) {
        initialize();  // Lazy initialization
    }

    Block* current = freeList;
    while (current != NULL) {
        if (current->free && current->size >= size) {
            // If block size perfectly fits or large enough to split
            if (current->size > size + (int)sizeof(Block)) {
                splitBlock(current, size);
            } else {
                current->free = 0;
            }
            // Return pointer to usable memory (after Block metadata)
            return (int*)((unsigned char*)current + sizeof(Block));
        }
        current = current->next;
    }

    // No suitable block found
    return NULL;
}

// Merge adjacent free blocks to avoid fragmentation
static void mergeFreeBlocks() {
    Block* current = freeList;
    while (current != NULL && current->next != NULL) {
        if (current->free && current->next->free) {
            current->size += sizeof(Block) + current->next->size;
            current->next = current->next->next;  // Remove merged block from list
        } else {
            current = current->next;
        }
    }
}

// Deallocate memory given a pointer allocated by allocate()
void deallocate(int* ptr) {
    if (!ptr) return;

    // Retrieve block metadata from pointer
    Block* block = (Block*)((unsigned char*)ptr - sizeof(Block));
    block->free = 1;

    // Try to merge free adjacent blocks to reduce fragmentation
    mergeFreeBlocks();
}

// ------------------------------
// Sample test cases demonstrating usage
// ------------------------------
int main() {
    int *mem[10];

    mem[0] = allocate(128);
    mem[1] = allocate(1024);
    mem[2] = allocate(4096);

    if (mem[0]) printf("mem[0] allocated 128 bytes\n");
    if (mem[1]) printf("mem[1] allocated 1024 bytes\n");
    if (mem[2]) printf("mem[2] allocated 4096 bytes\n");

    deallocate(mem[1]);
    printf("mem[1] deallocated\n");

    mem[1] = allocate(512);
    if (mem[1]) printf("mem[1] re-allocated 512 bytes\n");

    // Allocate maximum available size (close to 100KB)
    int *maxMem = allocate(100000);
    if (maxMem) {
        printf("Allocated nearly entire memory (100000 bytes)\n");
        deallocate(maxMem);
        printf("Deallocated max memory\n");
    } else {
        printf("Failed to allocate max memory (100000 bytes)\n");
    }

    return 0;
}