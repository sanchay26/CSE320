#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "helper.h"
#include "sfmm.h"

/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */

sf_free_header* freelist_head = NULL;


void *sf_malloc(size_t size){
	
	size_t alignedsize = alignsize(size)+16;

	size_t padding = alignedsize - size -16;

	void *bp;
	
	if(size <= 0 || size > ((4096*4)-16)){
		
		//Set Error Number
		return NULL;
	}
		
	else if(freelist_head == NULL){
			freelist_head = (sf_free_header*) sf_sbrk(1);
			freelist_head->header.alloc = 0;
			freelist_head->header.block_size = (4096>>4);
			freelist_head->header.padding_size = 0;
			freelist_head->next = NULL;
			freelist_head->prev = NULL;
			void *currentlocation = sf_sbrk(0);
			currentlocation = currentlocation + 4088;
			sf_footer *first_footer = (sf_footer*) currentlocation;
			first_footer->alloc = 0;
			first_footer->block_size = (4096>>4);
			
		}
	if((bp = firstFit(alignedsize))){
		placeFit(bp,alignedsize,padding);
		return bp+8;
	}
	
	
  return NULL;
}

void* firstFit(size_t alignedsize){

	sf_free_header* free;

	for(free = freelist_head; free->header.alloc == 0; free=free->next){

		if(alignedsize < (size_t)free->header.block_size)
		return free;
	}
	// Set the error flag.
	return NULL; //No fit found 
}

void placeFit( void *bp , size_t alignedsize, size_t padding){

	sf_free_header* oldfree = (sf_free_header*) bp;

	void * currentlocation = bp;
	//currentlocation = currentlocation + oldfree

	//setting the header to allocated.

	sf_header* fit = (sf_header*) bp;
	fit->alloc = 1;
	fit->block_size = alignedsize>>4;
	fit->padding_size = padding;
}


void sf_free(void *ptr){

}

void *sf_realloc(void *ptr, size_t size){
  return NULL;
}

int sf_info(info* meminfo){
  return -1;
}

size_t alignsize(size_t size){
	
	if(size%16 == 0){
		return size;
	}
	
	else
	{
		size_t remainder = size % 16 ; 
		return (size + 16 - remainder) ; 
	}
}
