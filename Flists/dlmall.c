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

struct head *flist;
struct head *flist_8;
struct head *flist_16;
struct head *flist_24;
struct head *flist_32;
struct head *flist_40;
struct head *flist_48;
struct head *flist_56;
struct head *flist_64;
struct head *flist_72;
struct head *flist_80;
struct head *flist_88;
struct head *flist_96;
struct head *flist_104;
struct head *flist_112;
struct head *flist_120;
struct head *flist_128;

struct head *midway;

void *new()
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

    // Initialise flists
    int list_size = 20;
    
    //8
    // enough for 8 blocks, and a sentinel
    uint size_8 = ((8 + HEAD) * list_size)  -  (HEAD);
    new->bfree = FALSE; // Cannot allocate here
    new->bsize = 0;
    new->free = TRUE; // memory is free
    new->size = size_8;
    flist_8 = new;
    arena = new;

    // Marks the end of the free list
    struct head *sentinel_8 = after(new);
    sentinel_8->bfree = TRUE; // memory is free
    sentinel_8->bsize = size_8;
    sentinel_8->free = FALSE; // Cannot allocate here
    sentinel_8->size = 0;

    //16
    // enough for 16 blocks, and a sentinel
    flist_16 = after(sentinel_8);
    uint size_16 = ((16 + HEAD) * list_size)  - (HEAD);
    flist_16->bfree = FALSE; // Cannot allocate here
    flist_16->bsize = 0;
    flist_16->free = TRUE; // memory is free
    flist_16->size = size_16;

    // Marks the end of the free list
    struct head *sentinel_16 = after(flist_16);
    sentinel_16->bfree = TRUE; // memory is free
    sentinel_16->bsize = size_16;
    sentinel_16->free = FALSE; // Cannot allocate here
    sentinel_16->size = 0;


    //24
    // enough for 24 blocks, and a sentinel
    flist_24 = after(sentinel_16);
    uint size_24 = ((24 + HEAD) * list_size)  - (HEAD);
    flist_24->bfree = FALSE; // Cannot allocate here
    flist_24->bsize = 0;
    flist_24->free = TRUE; // memory is free
    flist_24->size = size_24;

    // Marks the end of the free list
    struct head *sentinel_24 = after(flist_24);
    sentinel_24->bfree = TRUE; // memory is free
    sentinel_24->bsize = size_24;
    sentinel_24->free = FALSE; // Cannot allocate here
    sentinel_24->size = 0;

    //32
    // enough for 32 blocks, and a sentinel
    flist_32 = after(sentinel_24);
    uint size_32 = ((32 + HEAD) * list_size)  - (HEAD);
    flist_32->bfree = FALSE; // Cannot allocate here
    flist_32->bsize = 0;
    flist_32->free = TRUE; // memory is free
    flist_32->size = size_32;

    // Marks the end of the free list
    struct head *sentinel_32 = after(flist_32);
    sentinel_32->bfree = TRUE; // memory is free
    sentinel_32->bsize = size_32;
    sentinel_32->free = FALSE; // Cannot allocate here
    sentinel_32->size = 0;

    //40
    // enough for 40 blocks, and a sentinel
    flist_40 = after(sentinel_32);
    uint size_40 = ((40 + HEAD) * list_size)  - (HEAD);
    flist_40->bfree = FALSE; // Cannot allocate here
    flist_40->bsize = 0;
    flist_40->free = TRUE; // memory is free
    flist_40->size = size_40;

    // Marks the end of the free list
    struct head *sentinel_40 = after(flist_40);
    sentinel_40->bfree = TRUE; // memory is free
    sentinel_40->bsize = size_40;
    sentinel_40->free = FALSE; // Cannot allocate here
    sentinel_40->size = 0;

    //48
    // enough for 48 blocks, and a sentinel
    flist_48 = after(sentinel_40);
    uint size_48 = ((48 + HEAD) * list_size)  - (HEAD);
    flist_48->bfree = FALSE; // Cannot allocate here
    flist_48->bsize = 0;
    flist_48->free = TRUE; // memory is free
    flist_48->size = size_48;

    // Marks the end of the free list
    struct head *sentinel_48 = after(flist_48);
    sentinel_48->bfree = TRUE; // memory is free
    sentinel_48->bsize = size_48;
    sentinel_48->free = FALSE; // Cannot allocate here
    sentinel_48->size = 0;

    //56
    // enough for 56 blocks, and a sentinel
    flist_56 = after(sentinel_48);
    uint size_56 = ((56 + HEAD) * list_size)  - (HEAD);
    flist_56->bfree = FALSE; // Cannot allocate here
    flist_56->bsize = 0;
    flist_56->free = TRUE; // memory is free
    flist_56->size = size_56;

    // Marks the end of the free list
    struct head *sentinel_56 = after(flist_56);
    sentinel_56->bfree = TRUE; // memory is free
    sentinel_56->bsize = size_56;
    sentinel_56->free = FALSE; // Cannot allocate here
    sentinel_56->size = 0;

    //64
    // enough for 64 blocks, and a sentinel
    flist_64 = after(sentinel_56);
    uint size_64 = ((64 + HEAD) * list_size)  - (HEAD);
    flist_64->bfree = FALSE; // Cannot allocate here
    flist_64->bsize = 0;
    flist_64->free = TRUE; // memory is free
    flist_64->size = size_64;

    // Marks the end of the free list
    struct head *sentinel_64 = after(flist_64);
    sentinel_64->bfree = TRUE; // memory is free
    sentinel_64->bsize = size_64;
    sentinel_64->free = FALSE; // Cannot allocate here
    sentinel_64->size = 0;

    //72
    // enough for 72 blocks, and a sentinel
    flist_72 = after(sentinel_64);
    uint size_72 = ((72 + HEAD) * list_size)  - (HEAD);
    flist_72->bfree = FALSE; // Cannot allocate here
    flist_72->bsize = 0;
    flist_72->free = TRUE; // memory is free
    flist_72->size = size_72;


    // Marks the end of the free list
    struct head *sentinel_72 = after(flist_72);
    sentinel_72->bfree = TRUE; // memory is free
    sentinel_72->bsize = size_72;
    sentinel_72->free = FALSE; // Cannot allocate here
    sentinel_72->size = 0;

    //80
    // enough for 80 blocks, and a sentinel
    flist_80 = after(sentinel_72);
    uint size_80 = ((80 + HEAD) * list_size)  - (HEAD);
    flist_80->bfree = FALSE; // Cannot allocate here
    flist_80->bsize = 0;
    flist_80->free = TRUE; // memory is free
    flist_80->size = size_80;
    

    // Marks the end of the free list
    struct head *sentinel_80 = after(flist_80);
    sentinel_80->bfree = TRUE; // memory is free
    sentinel_80->bsize = size_80;
    sentinel_80->free = FALSE; // Cannot allocate here
    sentinel_80->size = 0;

    //88
    // enough for 88 blocks, and a sentinel
    flist_88 = after(sentinel_80);
    uint size_88 = ((88 + HEAD) * list_size)  - (HEAD);
    flist_88->bfree = FALSE; // Cannot allocate here
    flist_88->bsize = 0;
    flist_88->free = TRUE; // memory is free
    flist_88->size = size_88;
    

    // Marks the end of the free list
    struct head *sentinel_88 = after(flist_88);
    sentinel_88->bfree = TRUE; // memory is free
    sentinel_88->bsize = size_88;
    sentinel_88->free = FALSE; // Cannot allocate here
    sentinel_88->size = 0;

    //96
    // enough for 96 blocks, and a sentinel
    flist_96 = after(sentinel_88);
    uint size_96 = ((96 + HEAD) * list_size)  - (HEAD);
    flist_96->bfree = FALSE; // Cannot allocate here
    flist_96->bsize = 0;
    flist_96->free = TRUE; // memory is free
    flist_96->size = size_96;
    

    // Marks the end of the free list
    struct head *sentinel_96 = after(flist_96);
    sentinel_96->bfree = TRUE; // memory is free
    sentinel_96->bsize = size_96;
    sentinel_96->free = FALSE; // Cannot allocate here
    sentinel_96->size = 0;

    //104
    // enough for 104 blocks, and a sentinel
    flist_104 = after(sentinel_96);
    uint size_104 = ((104 + HEAD) * list_size)  - (HEAD);
    flist_104->bfree = FALSE; // Cannot allocate here
    flist_104->bsize = 0;
    flist_104->free = TRUE; // memory is free
    flist_104->size = size_104;
    

    // Marks the end of the free list
    struct head *sentinel_104 = after(flist_104);
    sentinel_104->bfree = TRUE; // memory is free
    sentinel_104->bsize = size_104;
    sentinel_104->free = FALSE; // Cannot allocate here
    sentinel_104->size = 0;

    //112
    // enough for 112 blocks, and a sentinel
    flist_112 = after(sentinel_104);
    uint size_112 = ((112 + HEAD) * list_size)  - (HEAD);
    flist_112->bfree = FALSE; // Cannot allocate here
    flist_112->bsize = 0;
    flist_112->free = TRUE; // memory is free
    flist_112->size = size_112;
    

    // Marks the end of the free list
    struct head *sentinel_112 = after(flist_112);
    sentinel_112->bfree = TRUE; // memory is free
    sentinel_112->bsize = size_112;
    sentinel_112->free = FALSE; // Cannot allocate here
    sentinel_112->size = 0;

    //120
    // enough for 120 blocks, and a sentinel
    flist_120 = after(sentinel_112);
    uint size_120 = ((120 + HEAD) * list_size)  - (HEAD);
    flist_120->bfree = FALSE; // Cannot allocate here
    flist_120->bsize = 0;
    flist_120->free = TRUE; // memory is free
    flist_120->size = size_120;
    

    // Marks the end of the free list
    struct head *sentinel_120 = after(flist_120);
    sentinel_120->bfree = TRUE; // memory is free
    sentinel_120->bsize = size_120;
    sentinel_120->free = FALSE; // Cannot allocate here
    sentinel_120->size = 0;

    //128
    // enough for 128 blocks, and a sentinel
    flist_128 = after(sentinel_120);
    uint size_128 = ((128 + HEAD) * list_size)  - (HEAD);
    flist_128->bfree = FALSE; // Cannot allocate here
    flist_128->bsize = 0;
    flist_128->free = TRUE; // memory is free
    flist_128->size = size_128;
    

    // Marks the end of the free list
    struct head *sentinel_128 = after(flist_128);
    sentinel_128->bfree = TRUE; // memory is free
    sentinel_128->bsize = size_128;
    sentinel_128->free = FALSE; // Cannot allocate here
    sentinel_128->size = 0;
    
    int mem_so_far = size_8 + size_16 + size_24 + size_32 + size_40 + size_48 + size_56 + size_64 + size_72 + size_80 + size_88 + size_96 + size_104 + size_112 + size_120 + size_128 + (32 * HEAD);

    // Make room for head and end-of-list dummy
    uint size = ARENA - mem_so_far - 2*HEAD;
    flist = after(sentinel_128);
    flist->bfree = FALSE; // Cannot allocate here
    flist->bsize = 0;
    flist->free = TRUE; // memory is free
    flist->size = size;

    // Marks the end of the free list
    struct head *sentinel = after(flist);
    sentinel->bfree = TRUE; // memory is free
    sentinel->bsize = size;
    sentinel->free = FALSE; // Cannot allocate here
    sentinel->size = 0;
    

    midway = (struct head*) flist;
}


struct head *flist;

// Used for detaching from the free list (not the same as allocating memory)

void detach(struct head *block, int flist_no)
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
        if(flist_no == 0)
        {
            flist = block->next;
        }
        else if(flist_no == 8)
        {
            flist_8 = block->next;
        }
        else if(flist_no == 16)
        {
            flist_16 = block->next;
        }
        else if(flist_no == 24)
        {
            flist_24 = block->next;
        }
        else if(flist_no == 32)
        {
            flist_32 = block->next;
        }
        else if(flist_no == 40)
        {
            flist_40 = block->next;
        }
        else if(flist_no == 48)
        {
            flist_48 = block->next;
        }
        else if(flist_no == 56)
        {
            flist_56 = block->next;
        }
        else if(flist_no == 64)
        {
            flist_64 = block->next;
        }
        else if(flist_no == 72)
        {
            flist_72 = block->next;
        }
        else if(flist_no == 80)
        {
            flist_80 = block->next;
        }
        else if(flist_no == 88)
        {
            flist_88 = block->next;
        }
        else if(flist_no == 96)
        {
            flist_96 = block->next;
        }
        else if(flist_no == 104)
        {
            flist_104 = block->next;
        }
        else if(flist_no == 112)
        {
            flist_112 = block->next;
        }
        else if(flist_no == 120)
        {
            flist_120 = block->next;
        }
        else if(flist_no == 128)
        {
            flist_128 = block->next;
        }
    }
    
}

// Used for inserting to free list (not the same as freeing memory)

void insert(struct head *block, int flist_no)
{
    
        if(flist_no == 0)
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
        else if(flist_no == 8)
        {
            block->next = NULL;
            block->prev = NULL;
            if (flist_8 != NULL)
            {
                block->next = flist_8;
                flist_8->prev = block;  
            }
            flist_8 = block;
        }
        else if(flist_no == 16)
        {
            block->next = NULL;
            block->prev = NULL;
            if (flist_16 != NULL)
            {
                block->next = flist_16;
                flist_16->prev = block;  
            }
            flist_16 = block;
        }
        else if(flist_no == 24)
        {
            block->next = NULL;
            block->prev = NULL;
            if (flist_24 != NULL)
            {
                block->next = flist_24;
                flist_24->prev = block;  
            }
            flist_24 = block;
        }
        else if(flist_no == 32)
        {
            block->next = NULL;
            block->prev = NULL;
            if (flist_32 != NULL)
            {
                block->next = flist_32;
                flist_32->prev = block;  
            }
            flist_32 = block;
        }
        else if(flist_no == 40)
        {
            block->next = NULL;
            block->prev = NULL;
            if (flist_40 != NULL)
            {
                block->next = flist_40;
                flist_40->prev = block;  
            }
            flist_40 = block;
        }
        else if(flist_no == 48)
        {
            block->next = NULL;
            block->prev = NULL;
            if (flist_48 != NULL)
            {
                block->next = flist_48;
                flist_48->prev = block;  
            }
            flist_48 = block;
        }
        else if(flist_no == 56)
        {
            block->next = NULL;
            block->prev = NULL;
            if (flist_56 != NULL)
            {
                block->next = flist_56;
                flist_56->prev = block;  
            }
            flist_56 = block;
        }
        else if(flist_no == 64)
        {
            block->next = NULL;
            block->prev = NULL;
            if (flist_64 != NULL)
            {
                block->next = flist_64;
                flist_64->prev = block;  
            }
            flist_64 = block;
        }
        else if(flist_no == 72)
        {
            block->next = NULL;
            block->prev = NULL;
            if (flist_72 != NULL)
            {
                block->next = flist_72;
                flist_72->prev = block;  
            }
            flist_72 = block;
        }
        else if(flist_no == 80)
        {
            block->next = NULL;
            block->prev = NULL;
            if (flist_80 != NULL)
            {
                block->next = flist_80;
                flist_80->prev = block;  
            }
            flist_80 = block;
        }
        else if(flist_no == 88)
        {
            block->next = NULL;
            block->prev = NULL;
            if (flist_88 != NULL)
            {
                block->next = flist_88;
                flist_88->prev = block;  
            }
            flist_88 = block;
        }
        else if(flist_no == 96)
        {
            block->next = NULL;
            block->prev = NULL;
            if (flist_96 != NULL)
            {
                block->next = flist_96;
                flist_96->prev = block;  
            }
            flist_96 = block;
        }
        else if(flist_no == 104)
        {
            block->next = NULL;
            block->prev = NULL;
            if (flist_104 != NULL)
            {
                block->next = flist_104;
                flist_104->prev = block;  
            }
            flist_104 = block;
        }
        else if(flist_no == 112)
        {
            block->next = NULL;
            block->prev = NULL;
            if (flist_112 != NULL)
            {
                block->next = flist_112;
                flist_112->prev = block;  
            }
            flist_112 = block;
        }
        else if(flist_no == 120)
        {
            block->next = NULL;
            block->prev = NULL;
            if (flist_120 != NULL)
            {
                block->next = flist_120;
                flist_120->prev = block;  
            }
            flist_120 = block;
        }
        else if(flist_no == 128)
        {
            block->next = NULL;
            block->prev = NULL;
            if (flist_128 != NULL)
            {
                block->next = flist_128;
                flist_128->prev = block;  
            }
            flist_128 = block;
        }

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

struct head *find(int size, int flist_no)
{
    struct head* to_alloc = NULL;
    // If the flist does not exist..
    if(flist_no == 0 && flist == NULL)
    {
        return NULL;
    }
    else
    {
        // While the flist is free (i.e. before we reach the sentinel)
        // Search list until we find a space big enough
        struct head* current;
        if(flist_no == 0)
        {
            current = flist;
        }
        else if(flist_no == 8)
        {
            current = flist_8;
        }
        else if(flist_no == 16)
        {
            current = flist_16;
        }
        else if(flist_no == 24)
        {
            current = flist_24;
        }
        else if(flist_no == 32)
        {
            current = flist_32;
        }
        else if(flist_no == 40)
        {
            current = flist_40;
        }
        else if(flist_no == 48)
        {
            current = flist_48;
        }
        else if(flist_no == 56)
        {
            current = flist_56;
        }
        else if(flist_no == 64)
        {
            current = flist_64;
        }
        else if(flist_no == 72)
        {
            current = flist_72;
        }
        else if(flist_no == 80)
        {
            current = flist_80;
        }
        else if(flist_no == 88)
        {
            current = flist_88;
        }
        else if(flist_no == 96)
        {
            current = flist_96;
        }
        else if(flist_no == 104)
        {
            current = flist_104;
        }
        else if(flist_no == 112)
        {
            current = flist_112;
        }
        else if(flist_no == 120)
        {
            current = flist_120;
        }
        else if(flist_no == 128)
        {
            current = flist_128;
        }

        while(current != NULL)
        {
            int c_size = current->size;
            if(c_size >= size)
            {
                // If we find a block large enough, detach it from free list
                to_alloc = current;
                detach(to_alloc, flist_no);
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
                insert(before(split_alloc), flist_no);
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

struct head *merge(struct head *block)
{
    struct head *aft = after(block);

    if(block->bfree)
    {
        struct head *bef = before(block);
        detach(bef, 0);
        int tot_size = block->size + bef->size + HEAD;
        bef->size = tot_size;
        aft->bsize = tot_size;
        aft->bfree = TRUE;

        block = bef;
    }

    if(aft->free)
    {
        detach(aft, 0);
        int size_tot = block->size + aft->size + HEAD;
        block->size = size_tot;
        struct head* aftaft = after(aft);
        aftaft->bsize = size_tot;
        aftaft->bfree = TRUE;
    }

    return block;
}

int flist_num(int size)
{
    if(size == 8)
    {
        if(flist_8 != NULL)
        {
            return 8;
        }
        else
        {
            return 0;
        }
    }
    else if (size == 16)
    {
        if(flist_16 != NULL)
        {
            return 16;
        }
        else
        {
            return 0;
        }
    }
    else if (size == 24)
    {
        if(flist_24 != NULL)
        {
            return 24;
        }
        else
        {
            return 0;
        }
    }
    else if (size == 32)
    {
        if(flist_32 != NULL)
        {
            return 32;
        }
        else
        {
            return 0;
        }
    }
    else if (size == 40)
    {
        if(flist_40 != NULL)
        {
            return 40;
        }
        else
        {
            return 0;
        }
    }
    else if (size == 48)
    {
        if(flist_48 != NULL)
        {
            return 48;
        }
        else
        {
            return 0;
        }
    }
    else if (size == 56)
    {
        if(flist_56 != NULL)
        {
            return 56;
        }
        else
        {
            return 0;
        }
    }
    else if (size == 64)
    {
        if(flist_64 != NULL)
        {
            return 64;
        }
        else
        {
            return 0;
        }
    }
    else if (size == 72)
    {
        if(flist_72 != NULL)
        {
            return 72;
        }
        else
        {
            return 0;
        }
    }
    else if (size == 80)
    {
        if(flist_80 != NULL)
        {
            return 80;
        }
        else
        {
            return 0;
        }
    }
    else if (size == 88)
    {
        if(flist_88 != NULL)
        {
            return 88;
        }
        else
        {
            return 0;
        }
    }
    else if (size == 96)
    {
        if(flist_96 != NULL)
        {
            return 96;
        }
        else
        {
            return 0;
        }
    }
    else if (size == 104)
    {
        if(flist_104 != NULL)
        {
            return 104;
        }
        else
        {
            return 0;
        }
    }
    else if (size == 112)
    {
        if(flist_112 != NULL)
        {
            return 112;
        }
        else
        {
            return 0;
        }
    }
    else if (size == 120)
    {
        if(flist_120 != NULL)
        {
            return 120;
        }
        else
        {
            return 0;
        }
    }
    else if (size == 128)
    {
        if(flist_128 != NULL)
        {
            return 128;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
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
    int flist_no = flist_num(size);
    struct head *taken = find(size, flist_no);
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

        //struct head *aft = flist;
        block->free = TRUE;

        if(block >= midway)
        {
            struct head *mergey;
            mergey = merge(block);
            insert(mergey, 0);
        }
        else
        {
            insert(block, block->size);
        }
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
        printf("My size is %d\n", current->size);
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
        new();
    }
}

int sanity_flists(struct head* flisty)
{
    int length;
    int acc_size;
    acc_size = 0;
    length = 0;
    struct head* current = flisty;
    while(current != NULL)
    {
        acc_size = acc_size + current->size;
        current = current->next;
        length ++;
    }
    printf("Length of the free list: %d\n", length);
    printf("Total size of free list nodes: %d\n", acc_size);
    if(flisty != NULL)
    {
        printf("Average size of free list nodes: %d\n", acc_size / length);
    }
    printf("\n");

    return length;

}

void init_sanity_flists()
{
    int sum = 0;   
    sum +=  sanity_flists(flist_16);
    sum +=  sanity_flists(flist_24);
    sum += sanity_flists(flist_32);
    sum += sanity_flists(flist_40);
    sum += sanity_flists(flist_48);
    sum += sanity_flists(flist_56);
    sum += sanity_flists(flist_64);
    sum += sanity_flists(flist_72);
    sum += sanity_flists(flist_80);
    sum += sanity_flists(flist_88);
    sum += sanity_flists(flist_96);
    sum += sanity_flists(flist_104);
    sum += sanity_flists(flist_112);
    sum += sanity_flists(flist_120);
    sum += sanity_flists(flist_128);
    sum += sanity_flists(flist);
    printf("%d\n", sum);
}


