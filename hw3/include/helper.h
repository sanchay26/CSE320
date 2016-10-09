#include <stdio.h>


size_t alignsize(size_t size);

void* firstFit(size_t alignedsize);

void placeFit(void *bp,size_t alignedsize,size_t padding);