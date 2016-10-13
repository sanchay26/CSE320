#include <stdio.h>


size_t alignsize(size_t size);

void* firstFit(size_t alignedsize);

void placeFit(void *bp,size_t alignedsize,size_t padding);

void setheader(void *bp, size_t alignedsize, size_t padding);

void setfooter(void *bp, size_t alignedsize);

void setfreeheader(void *bp, size_t alignedsize);

void setfreefooter(void *bp, size_t alignedsize);

void removeBlock(void *bp);

void coalesce(void *bp);

void* next_block(void *bp);

void* prev_block(void *bp);

size_t getSize(void *bp);
size_t getAlloc(void *bp);

void insertatfront(void *bp);

void printblocks();