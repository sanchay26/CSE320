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

void *startheap;
void *endheap;
int begin = 0;

void *sf_malloc(size_t size){
	
	//Aligning the size

	size_t alignedsize = alignsize(size)+16;

	// getting the padded size.
	size_t padding = alignedsize - size -16;

	void *bp = NULL;
	
	if(size <= 0 || size > ((4096*4)-16)){
		
		//Set Error Number
		return NULL;
	}
		
	else if(begin == 0){
			endheap = sf_sbrk(1);
			freelist_head = (sf_free_header*) (endheap-4096);
			startheap = (endheap - 4096);
			freelist_head->header.alloc = 0;
			freelist_head->header.block_size = (4096>>4);
			freelist_head->header.padding_size = 0;
			freelist_head->next = NULL;
			freelist_head->prev = NULL;
			void *currentlocation = endheap - 8;
			sf_footer *first_footer = (sf_footer*) currentlocation;
			first_footer->alloc = 0;
			first_footer->block_size = (4096>>4);
			begin =1;
	}

	while (bp == NULL){

		bp = firstFit(alignedsize);

		if(bp == NULL){

			endheap = sf_sbrk(1);
			if(endheap == NULL){
				//--------------------------------------Set Error Number---------------------------------
			}

			void *location = endheap - 4096;
			setfreeheader(location,4096);
			setfreefooter(endheap-8,4096);
			coalesce(location);

		}

		else{
			placeFit(bp,alignedsize,padding);
			return bp+8;}
		}
	
	// if((bp = firstFit(alignedsize))){
	// 	placeFit(bp,alignedsize,padding);
	// 	return bp+8;
	// }
	
	return NULL;
}

void* firstFit(size_t alignedsize){

	sf_free_header* free;


	for(free = freelist_head; free!=NULL; free=free->next){

		if(free == NULL){
			return NULL;
		}

		if(alignedsize <= (size_t)free->header.block_size<<4)
		return free;
	}
	//fit not found ask for new page.
	// Set the error flag.
	return NULL; //No fit found 
}

void placeFit( void *bp , size_t alignedsize, size_t padding){

	sf_free_header* oldfree = (sf_free_header*) bp;

	size_t freesize = oldfree->header.block_size<<4;

	//split the blocks

	if((freesize-alignedsize) >= 32){
		setheader(bp,alignedsize,padding);
		setfooter(bp,alignedsize);
		removeBlock(bp);
		bp = bp + alignedsize;
		setfreeheader(bp,(freesize-alignedsize));
		setfreefooter(bp,(freesize-alignedsize));
		coalesce(bp);
		
	}
	
	else if((freesize-alignedsize)==0){
		if(oldfree == freelist_head && freelist_head->next == NULL){
			printf("%s\n","I am here" );
			setheader(bp,alignedsize,padding);
			setfooter(bp,alignedsize);
			freelist_head =NULL;
		}
		
	}

	else{
		//Splinters And no space to store anything 
	}
}

void setheader(void *bp, size_t alignedsize, size_t padding){
	sf_header* fit = (sf_header*) bp;
	fit->alloc = 1;
	fit->block_size = alignedsize>>4;
	fit->padding_size = padding;
}

void setfreeheader(void *bp, size_t alignedsize){
	sf_free_header* fit = (sf_free_header*) bp;
	fit->header.alloc = 0;
	fit->header.block_size = alignedsize>>4;
	fit->header.padding_size = 0;
	//freelist_head = fit;
}

void setfooter(void *bp, size_t alignedsize){
	void * currentlocation = bp;
	currentlocation = currentlocation+alignedsize-8;
	sf_footer* footer = (sf_footer*) currentlocation;
	footer->alloc = 1;
	footer->block_size = alignedsize>>4;
}

void setfreefooter(void *bp, size_t alignedsize){
	void * currentlocation = bp;
	currentlocation = currentlocation +(alignedsize)-8;
	sf_footer* footer = (sf_footer*) currentlocation;
	footer->alloc = 0;
	footer->block_size = alignedsize>>4;
}


void sf_free(void *ptr){

	if(!ptr) return;

	void* cloneptr = ptr-8;

	sf_header *free = (sf_header*)cloneptr;

	size_t freesize = free->block_size<<4;

	setfreeheader(cloneptr, freesize);

	setfreefooter(cloneptr, freesize);

	coalesce(cloneptr);

	// sf_free_header *ptr1= (sf_free_header*)ptr;
	// freelist_head->prev = ptr1;
	// ptr1->next = freelist_head;
	// freelist_head =ptr1;

}


void insertatfront(void *bp){

	sf_free_header *insert = (sf_free_header*)bp;
	
	if(freelist_head == NULL){
		freelist_head = insert;
		freelist_head->prev = NULL;
		freelist_head->next = NULL;
	}
	
	else{
	insert->next = freelist_head;
	freelist_head->prev = insert;
	insert->prev = NULL;
	freelist_head = insert;
	}

}

void coalesce(void *bp){

	
	sf_free_header* freed = (sf_free_header*)bp;

	size_t prev_alloc = getAlloc(prev_block(bp)) || bp ==startheap;
	
	size_t next_alloc = getAlloc(next_block(bp)) || ((bp + getSize(bp) - SF_FOOTER_SIZE )== (endheap-8));


	size_t size = freed->header.block_size<<4;
	
	/*case1*/
	if(prev_alloc && !next_alloc){
		printf("%s\n","In case 1");
		size = size + getSize(next_block(bp));
		removeBlock(next_block(bp));
		setfreeheader(bp,size); 
		setfreefooter(bp,size);
	}
	/*case2*/
	else if(!prev_alloc && next_alloc){
		printf("%s\n","In case 2");
		size = size + getSize(prev_block(bp));
		bp = prev_block(bp);
		removeBlock(bp);
		setfreeheader(bp,size); 
		setfreefooter(bp,size);
	}
	/*case3*/

	if(!prev_alloc && !next_alloc){
		printf("%s\n","In case 3");
		size = size + getSize(prev_block(bp))+ getSize(next_block(bp));
		removeBlock(prev_block(bp));
		removeBlock(next_block(bp));
		bp = prev_block(bp);
		setfreeheader(bp,size);
		setfreefooter(bp,size);
	}

	insertatfront(bp);
	//printblocks();
}

void* next_block(void *bp){
	int *next = (int*)bp;
	size_t size = *next & 0xfffffff0;
	void *nextblock = bp+size;
	printf("%p\n",next_block);
	return nextblock;
}

void* prev_block(void *bp){
	int *prev = (int*)(bp-8);
	size_t size = *prev & 0xfffffff0;
	void *prevblock = bp-size;
	printf("%p\n",prev_block);
	return prevblock;
}

size_t getAlloc(void *bp){
	int *kptr = (int*)bp;
	size_t alloc = *kptr & 0x1;
	return alloc;
}

size_t getSize(void *bp){
	int *kptr = (int*)bp;
	size_t size = *kptr & 0xfffffff0;
	return size;
}

void removeBlock(void *bp){

	printf("%s\n","in remove block" );
	sf_free_header *remove = (sf_free_header*)bp;

	sf_free_header *previousfree = remove->prev;

	sf_free_header *nextfree = remove->next;

	if(previousfree!=NULL){
		previousfree->next = nextfree;
	}
	else{
		freelist_head = nextfree;
	}
	if(nextfree!=NULL){
		nextfree->prev = previousfree;	
	}
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

void printblocks(){
	void *ptr = startheap;
	printf("%s\n","-----------------------------------------------------------------------------------------------------" );
	while(ptr<endheap){
		
		sf_blockprint(ptr);
		sf_header *h2= (sf_header*) ptr;
		ptr = ptr + (h2->block_size<<4);

	}
	printf("%s\n","-----------------------------------------------------------------------------------------------------" );
}