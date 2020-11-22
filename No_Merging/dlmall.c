#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

// Some definitions for future use:
// HEAD is important, as we can reference the size of a header easily

// MIN() is the minimum size that we will hand out. The minimum size, apart
// from the header, that a block will consist of. Currently, it is set to 8 bytes

// LIMIT() is the size that a block has to be larger than in order to split it.
// For example, if we want to split a block to accommodate 32 bytes, the block must
// be LIMIT(32) = 8 + 24 + 32

// MACIC() and HIDE() are used as a way of hiding and retrieving the header

// ALIGN reminds us that memory which is returned needs to be aligned with 8 bytes, on a 64 bit architecture

// ARENA is a large block which we allocate at the beginning, i.e the whole 64 kbyte heap.
#define TRUE 1
#define FALSE 0
#define HEAD (sizeof(struct head))
#define MIN(size) (((size)>(8))?(size):(8))
#define LIMIT(size) (MIN(0) + HEAD + size)
#define MAGIC(memory) ((struct head*) memory - 1)
#define HIDE(block) (void*)((struct head*) block + 1)
#define ALIGN 8
#define ARENA (64*1024)

// Implementation of a block header in the free list
// The block header must be aligned to a multiple of 8 bytes
// We want to keep the size of this header as small as possible, since it is overhead.
// Currently, the header size is 24 bytes.
struct head
{
    uint16_t bfree; // 2 bytes, the status of the block before
    uint16_t bsize; // 2 bytes, the size of the block before
    uint16_t free; // 2 bytes, the status of this block
    uint16_t size; // 2 bytes, the size of this block (max size is 2^16, that is 64 kbytes)
    struct head *next; // 8 bytes, pointer for free list?
    struct head *prev; // 8 bytes, pointer for free list?
};

// The size information of a block, given in the header, will allow us
// to determine where the block after it is located.
// The following function will return a pointer to the block after the block given
// as an argument
struct head *after(struct head *block)
{
    // To do pointer arithmetic, must convert to char*
    
    char *p = (char*)block;
    return (struct head*) (p + HEAD + (block->size));
}

// Similarly, this returns the previous block's header
struct head *before(struct head *block)
{
    char *p = (char*)block;
    return (struct head*) (p - HEAD - (block->bsize));
}

// We also need a procedure that given a (large enough) block and a size, splits
// the block in tow giving us a pointer to the second block.
struct head *split(struct head *block, int size)
{
    // Here we calculate the size of the freelist block, minus the size requested and a header

    int remaining_size = block->size - (size + HEAD);
    block->size = remaining_size;

    // Creating Allocated Memory
    struct head *splt = after(block); // THIS MAY BE WRONG
    splt->bsize = remaining_size; // size of block before
    splt->bfree = TRUE; // Free
    splt->size = size; // sie of this block
    splt->free = FALSE; // Allocated

    // update the size of the next block in the free list
    // THIS IS CREATING PROBELMS!!
    struct head *aft = after(splt);
    aft->bsize = splt->size;

    return splt;
}

// Creating new blocks can be done with mmap(). This process will allocate
// Memory for our process.
struct head *arena = NULL;

struct head *new()
{
    if(arena != NULL)
    {
        printf("One arena already allocated\n");
        return NULL;
    }
    // Using mmap, but we could have also used sbrk
    struct head *new = mmap(NULL, ARENA, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(new == MAP_FAILED)
    {
        printf("mmap failed");
        return NULL;
    }

    // Make room for head and end-of-list dummy
    uint size = ARENA - 2*HEAD;
    new->bfree = FALSE; // Cannot allocate here
    new->bsize = 0;
    new->free = TRUE; // memory is free
    new->size = size;

    // Marks the end of the free list
    struct head *sentinel = after(new);
    sentinel->bfree = TRUE; // memory is free
    sentinel->bsize = size;
    sentinel->free = FALSE; // Cannot allocate here
    sentinel->size = 0;
    

    arena = (struct head*) new;
    return new;
}

struct head *flist;

// Used for detaching from the free list (not the same as allocating memory)

void detach(struct head *block)
{
    if(block->next != NULL)
    {
        block->next->prev = block->prev;
    }
    if(block->prev != NULL)
    {
        block->prev->next = block->next;
    }
    else
    {
        //block = NULL;
        // If you're removing the first block?
        flist = block->next;
    }
    
}

// Used for inserting to free list (not the same as freeing memory)

void insert(struct head *block)
{
    block->next = NULL;
    block->prev = NULL;
    if (flist != NULL)
    {
        block->next = flist;
        flist->prev = block;  
    }
    flist = block;
}

int adjust (size_t request)
{
    // Adjust the request to a multiple of ALIGN
    int adj = request / ALIGN;
    int rem = request % ALIGN;
    int res;
    if(rem == 0)
    {
        res = request;
    }
    else
    {
        res = (adj * ALIGN) + ALIGN;
    }

    return MIN(res);
}

struct head *find(int size)
{
    struct head* to_alloc = NULL;
    // If the flist does not exist..
    if(flist == NULL)
    {
        return NULL;
    }
    else
    {
        // While the flist is free (i.e. before we reach the sentinel)
        // Search list until we find a space big enough
        struct head* current = flist;
        while(current != NULL)
        {
            int c_size = current->size;
            if(c_size >= size)
            {
                // If we find a block large enough, detach it from free list
                to_alloc = current;
                detach(to_alloc);
                break;
            }
            else
            {
                current = current->next;
            }
        }

        // If we have not found anything big enough, return NULL
        if (to_alloc == NULL)
        {
            return NULL;
        }
        else
        {
            // if the block we have found it big enough to split
            if(to_alloc->size >= LIMIT(size))
            {
                // Split it
                struct head* split_alloc = split(to_alloc, size);
                // Reattach the unused memory back onto free list
                insert(before(split_alloc));
                after(split_alloc)->bfree = FALSE;
                return split_alloc;
            }
            else
            {
                // Mark the allocated space as not free
                to_alloc->free = FALSE;
                after(to_alloc)->bfree = FALSE;
                return to_alloc;
            }
        }
    }
}

void *dalloc(size_t request)
{
    if (request <= 0)
    {
        printf("Invalid Dalloc Request");
        return NULL;
    }
    int size = adjust(request);
    struct head *taken = find(size);
    if(taken == NULL)
    {
        return NULL;
    }
    else
    {
        return HIDE(taken);
    }
}

// Currently, this is a cheat method as we are just reinserting ablock in the free list (no merging)
void dfree(void *memory)
{
    if(memory != NULL)
    {
        struct head * block = (struct head*) MAGIC(memory);

        struct head *aft = flist;
        block->free = TRUE;
        //aft->bfree = TRUE;
        insert(block);

    }
    return;
}

// Checks that the free list is ok
void sanity()
{
    int length;
    int acc_size;
    acc_size = 0;
    length = 0;
    struct head* current = flist;
    while(current != NULL)
    {
        printf("I am: %p\n", current);
        printf("flist node free? expected result 1: %d\n", current->free);
        printf("flist node is divisible size? expected result 0: %d\n", (current->size) % ALIGN);
        printf("node prev: %p\n", current->prev);
        printf("node next: %p\n", current->next);
        printf("\n");
        acc_size = acc_size + current->size;
        current = current->next;
        length ++;
    }
    printf("Length of the free list: %d\n", length);
    printf("Total size of free list nodes: %d\n", acc_size);
    printf("Average size of free list nodes: %d\n", acc_size / length);
}

void traverse()
{
    struct head* current = arena;
    char * end = (char*)arena + ARENA;
    while((char*)current < end)
    {
        printf("I am: %p\n", current);
        printf("memory node free? 1 is free, 0 is not free: %d\n", current->free);
        printf("memory node is divisible size? expected result 0: %d\n", (current->size) % ALIGN);
        printf("My memory size is : %d\n", current->size);
        printf("The previous memory size is : %d\n", current->bsize);
        printf("prev in memory: %p\n", before(current));
        printf("next in memory: %p\n", after(current));
        printf("\n");
        current = after(current);
    }
}

int initiated = FALSE;
void init()
{
    if (!initiated)
    {
        initiated = TRUE;
        flist = new();
    }
}


