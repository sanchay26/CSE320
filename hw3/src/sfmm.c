#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "helper.h"
#include <string.h>
#include "sfmm.h"

/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */

sf_free_header* freelist_head = NULL;

size_t totalfrees= 0;
size_t totalcoalesce = 0;
size_t totalmalloc = 0;
size_t external = 0;
size_t internal = 0;

void *startheap;
void *endheap;
int begin = 0;

void *sf_malloc(size_t size){
	
	//Aligning the size

	size_t alignedsize = alignsize(size)+16;

	// getting the padded size.
	size_t padding = alignedsize - size -16;

	void *bp = NULL;
	
	if(size <= 0){
		errno = EINVAL;
		return NULL;
	}
	
	else if(size > ((4096*4)-16)){
		errno = ENOMEM;
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
				errno = ENOMEM;
				return NULL;
			}

			void *location = endheap - 4096;
			setfreeheader(location,4096);
			setfreefooter(endheap-8,4096);
			coalesce(location);

		}

		else{
			placeFit(bp,alignedsize,padding);
			totalmalloc = totalmalloc+1;
			return bp+8;
		}
	
	}
	errno = ENOMEM;
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
			setheader(bp,alignedsize,padding);
			setfooter(bp,alignedsize);
			freelist_head =NULL;
		}
		
	}

	else if((freesize-alignedsize)<32){
		size_t newsize = alignedsize +(freesize-alignedsize);
		setheader(bp,newsize,padding);
		setfooter(bp,newsize);
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

	if(cloneptr < startheap || cloneptr > endheap){
		errno = EINVAL;
		return;
	}

	if(free->alloc == 0){
		errno = EINVAL;
		return;
	}

	size_t freesize = free->block_size<<4;

	setfreeheader(cloneptr, freesize);

	setfreefooter(cloneptr, freesize);

	coalesce(cloneptr);

	totalfrees = totalfrees +1;

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
		size = size + getSize(next_block(bp));
		removeBlock(next_block(bp));
		setfreeheader(bp,size); 
		setfreefooter(bp,size);
		totalcoalesce = totalcoalesce +1;
	}
	/*case2*/
	else if(!prev_alloc && next_alloc){
		size = size + getSize(prev_block(bp));
		bp = prev_block(bp);
		removeBlock(bp);
		setfreeheader(bp,size); 
		setfreefooter(bp,size);
		totalcoalesce = totalcoalesce +1;
	}
	/*case3*/

	if(!prev_alloc && !next_alloc){
		size = size + getSize(prev_block(bp))+ getSize(next_block(bp));
		removeBlock(prev_block(bp));
		removeBlock(next_block(bp));
		bp = prev_block(bp);
		setfreeheader(bp,size);
		setfreefooter(bp,size);
		totalcoalesce = totalcoalesce +1;
	}

	insertatfront(bp);
}

void* next_block(void *bp){
	int *next = (int*)bp;
	size_t size = *next & 0xfffffff0;
	void *nextblock = bp+size;
	return nextblock;
}

void* prev_block(void *bp){
	int *prev = (int*)(bp-8);
	size_t size = *prev & 0xfffffff0;
	void *prevblock = bp-size;
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

	size_t oldsize;

	size_t newsize;

	void *newptr;

	
	oldsize = getSize(ptr-8);

	newsize = alignsize(size)+16;

	size_t padding = newsize - size - 16;

	if(size <= 0 || ptr == NULL){
		errno = EINVAL;
		return NULL;
	}

	if( ((ptr-8) < startheap) || ((ptr-8) > endheap)){
		errno = EINVAL;
		return NULL;
	}

	if(getAlloc(ptr-8) == 0){
		errno = EINVAL;
		return NULL;
	}

	if(newsize == oldsize){
		return ptr;
	}

	if(newsize < oldsize){

		if(oldsize - newsize < 32){
			return ptr;
		}
		setheader(ptr-8,newsize,padding);
		setfooter(ptr-8,newsize);
		void *location = (ptr-8)+newsize;
		setfreeheader(location,(oldsize-newsize));
		setfreefooter(location,(oldsize-newsize));
		coalesce(location);
		return ptr;
	}

	size_t nextalloc = getAlloc(next_block(ptr-8));
	size_t nextsize = getSize(next_block(ptr-8));
	
	if(nextalloc == 0 && nextsize+oldsize>= newsize){
		
		if(((nextsize+oldsize)-newsize) <32){
		setheader(ptr-8,nextsize+oldsize,padding);
		setfooter(ptr-8,nextsize+oldsize);
		return ptr;
		}
		
		else{
			setheader(ptr-8,newsize,padding);
			setfooter(ptr-8,newsize);
			void *location = (ptr-8)+newsize;
			removeBlock(location);
			setfreeheader(location,(oldsize+nextsize-newsize));
			setfreefooter(location,(oldsize+nextsize-newsize));
			coalesce(location);
			return ptr;
		}
	}
	
	else if(nextalloc == 0 && nextsize+oldsize < newsize){
		newptr = sf_malloc(size);
		if(newptr == NULL){
			errno = ENOMEM;
			return NULL;
		}
		memcpy(newptr,ptr,oldsize-16);
		sf_free(ptr);
		return newptr;

	}
	
	else if(nextalloc == 1){
		newptr = sf_malloc(size);

		if(newptr == NULL){
			errno = ENOMEM;
			return NULL;
		}
		
		memcpy(newptr,ptr,oldsize-16);
		sf_free(ptr);
		return newptr;
	}
	
return NULL;
}

int sf_info(info* meminfo){

	if(meminfo != NULL){

		meminfo->allocations = totalmalloc;
		meminfo->frees = totalfrees;
		meminfo->coalesce = totalcoalesce;
		meminfo->external = calculateExternal();
		meminfo->internal = calculateInternal();
		return 0;
	}

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

size_t calculateExternal(){

	external = 0;
	sf_free_header *externalfree = freelist_head;

	if(externalfree == NULL){
		return 0;
	}
	for (externalfree = freelist_head; externalfree!=NULL; externalfree = externalfree->next){
		external = external + (externalfree->header.block_size<<4);	
	}

	return external;

}

size_t calculateInternal(){

	internal = 0;

	void *count = startheap;

	if(count == NULL){
		return 0;
	}
	
	while(count < endheap){

		sf_header *help = (sf_header*)count;

		if(help->alloc == 1){

			internal = internal + 16 + (help->padding_size);

		}

		count = count + (help->block_size<<4);
	}
	return internal;
}