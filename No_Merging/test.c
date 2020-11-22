#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <time.h>
#include "dlmall.h"

#define REQ_UPPER 5 // upper number of dalloc requests/frees at once
#define REQ_LOWER 1 // lower number of dalloc requests/frees at once

void test1(int upper)
{
    // first request will always work
    srand(time(NULL));
    int i = 1;
    int randomnumber;
    randomnumber = (rand() % upper) + 1;

    struct head *alloc = dalloc(randomnumber);
    printf("%d SUCCESS: %d bytes allocated. %p\n",i, randomnumber, alloc);
    i++;
    while(alloc != NULL)
    {
        randomnumber = (rand() % upper) + 1;
        alloc = dalloc(randomnumber);
        if(alloc != NULL)
        {
            printf("%d SUCCESS: %d bytes allocated. %p\n",i, randomnumber, alloc);
            i++;
        }
        else
        {
            printf("%d FAILURE\n", i);
            i++;
        }
    }
}

void test2(volatile int loops, int av)
{
    // let's use 100 indices
    struct head* procs[100]; 
    int i;
    for (i = 0; i < 100; i ++)
    {
        procs[i] = NULL;
    }

    srand(time(NULL));
    while(loops > 0)
    { 
        int randomnumber;
        //printf("%d ", loops);
        randomnumber = (rand() % 100);

        if(procs[randomnumber] == NULL)
        {
            
            int randalloc;
            randalloc = (rand() % av) + 1;
            struct head *temp = dalloc(randalloc);
            if(temp == NULL)
            {
                //printf("MALLOC FAILED");
            }
            else
            {
                procs[randomnumber] = temp;
            }
        }
        else
        {
            struct head *temp = procs[randomnumber];
            if(temp != NULL)
            {
                dfree(temp);
            }
            procs[randomnumber] = NULL;
        }
        loops --;
    }

    //sanity();
    //traverse();

}

int main()
{
    // Initialise our program memory
    init();

    //Perform tests as appropriate, e.g.
    clock_t start, end;
    double cpu_time_used;

    start = clock();
    test2(100,50);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("I took: %f", cpu_time_used);
    return 0;
}