#include <criterion/criterion.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sfmm.h"

/**
 *  HERE ARE OUR TEST CASES NOT ALL SHOULD BE GIVEN STUDENTS
 *  REMINDER MAX ALLOCATIONS MAY NOT EXCEED 4 * 4096 or 16384 or 128KB
 */

Test(sf_memsuite, Malloc_an_Integer, .init = sf_mem_init, .fini = sf_mem_fini) {
    int *x = sf_malloc(sizeof(int));
    *x = 4;
    cr_assert(*x == 4, "Failed to properly sf_malloc space for an integer!");
}

Test(sf_memsuite, Free_block_check_header_footer_values, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *pointer = sf_malloc(sizeof(short));
    sf_free(pointer);
    pointer = pointer - 8;
    sf_header *sfHeader = (sf_header *) pointer;
    cr_assert(sfHeader->alloc == 0, "Alloc bit in header is not 0!\n");
    sf_footer *sfFooter = (sf_footer *) (pointer - 8 + (sfHeader->block_size << 4));
    cr_assert(sfFooter->alloc == 0, "Alloc bit in the footer is not 0!\n");
}

Test(sf_memsuite, PaddingSize_Check_char, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *pointer = sf_malloc(sizeof(char));
    pointer = pointer - 8;
    sf_header *sfHeader = (sf_header *) pointer;
    cr_assert(sfHeader->padding_size == 15, "Header padding size is incorrect for malloc of a single char!\n");
}

Test(sf_memsuite, Check_next_prev_pointers_of_free_block_at_head_of_list, .init = sf_mem_init, .fini = sf_mem_fini) {
    int *x = sf_malloc(4);
    memset(x, 0, 4);
    cr_assert(freelist_head->next == NULL);
    cr_assert(freelist_head->prev == NULL);
}

Test(sf_memsuite, Coalesce_no_coalescing, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *x = sf_malloc(4);
    void *y = sf_malloc(4);
    memset(y, 0xFF, 4);
    sf_free(x);
    cr_assert(freelist_head == x-8);
    sf_free_header *headofx = (sf_free_header*) (x-8);
    sf_footer *footofx = (sf_footer*) (x - 8 + (headofx->header.block_size << 4)) - 8;

    sf_blockprint((sf_free_header*)((void*)x-8));
    // All of the below should be true if there was no coalescing
    cr_assert(headofx->header.alloc == 0);
    cr_assert(headofx->header.block_size << 4 == 32);
    cr_assert(headofx->header.padding_size == 0);

    cr_assert(footofx->alloc == 0);
    cr_assert(footofx->block_size << 4 == 32);
}

/*
//############################################
// STUDENT UNIT TESTS SHOULD BE WRITTEN BELOW
// DO NOT DELETE THESE COMMENTS
//############################################
*/

Test(sf_memsuite, Zero_Size_Allocation, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* x = sf_malloc(0);
    cr_assert(x == NULL);
}

Test(sf_memsuite, One_Full_Page_Allocation, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* x = sf_malloc(4080);
    memset(x,1,4080);
    info inf;
    sf_info(&inf);
    cr_assert(inf.internal == 16);
    cr_assert(inf.external == 0);
    cr_assert(inf.allocations== 1);
}

Test(sf_memsuite, Two_Full_Page_Allocation, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* x = sf_malloc(8176);
    memset(x,1,8176);
    cr_assert(freelist_head == NULL);
}

Test(sf_memsuite, Two_Page_Allocation, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* x = sf_malloc(4097);
    memset(x,1,4097);
    sf_free(x);
    cr_assert(freelist_head->header.block_size << 4 == (4096*2));
}

Test(sf_memsuite, Three_Page_Allocation, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* x = sf_malloc(8193);
    memset(x,1,8193);
    sf_free(x);
    cr_assert(freelist_head->header.block_size << 4 == (4096*3));
}

Test(sf_memsuite, Four_Page_Allocation, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* x = sf_malloc(4096*3+1);
    memset(x,1,4096*3+1);
    sf_free(x);
    cr_assert(freelist_head->header.block_size << 4 == (4096*4));
}

Test(sf_memsuite, Full_Page_Allocation_and_free, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *x = sf_malloc(4080);
    memset(x, 2, 4000);
    cr_assert(freelist_head == NULL);
    sf_free(x);
    cr_assert(freelist_head->header.block_size << 4 == 4096);
}

Test(sf_memsuite, Splinter_Allocation_and_free, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *x = sf_malloc(32); // Block will be 32 + 16
    void* y = sf_malloc(4016); // Free block is 4048 before malloc
    memset(x, 1, 32);
    memset(y, 2, 4000);
    cr_assert(((sf_header*)(y-8))->block_size << 4 == 4048);
}

Test(sf_memsuite, Coalesce_prev_block, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *w = sf_malloc(32);
    void *x = sf_malloc(64);    
    void *y = sf_malloc(96);
    void *z = sf_malloc(128);
    memset(w, 1, 32);
    memset(x, 2, 64);
    memset(y, 3, 96);
    memset(z, 4, 128);
    cr_assert(freelist_head==(z-8+144));
    sf_free(x);
    sf_free(y);
    cr_assert(freelist_head==(x-8));
    cr_assert(freelist_head->header.padding_size == 0);
    cr_assert(freelist_head->header.block_size << 4 == 192);
    cr_assert(freelist_head->next == (z-8+144));
    cr_assert(freelist_head->prev == NULL);
}

Test(sf_memsuite, Coalesce_next_block, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *w = sf_malloc(32);
    void *x = sf_malloc(64);    
    void *y = sf_malloc(96);
    void *z = sf_malloc(128);
    memset(w, 1, 32);
    memset(x, 2, 64);
    memset(y, 3, 96);
    memset(z, 4, 128);
    cr_assert(freelist_head==(z-8+144));
    sf_free(y);
    sf_free(x);
    cr_assert(freelist_head==(x-8));
    cr_assert(freelist_head->header.padding_size == 0);
    cr_assert(freelist_head->header.block_size << 4 == 192);
    cr_assert(freelist_head->next == (z-8+144));
    cr_assert(freelist_head->prev == NULL);
}

Test(sf_memsuite, Coalesce_both_block_1, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *w = sf_malloc(32);
    void *x = sf_malloc(64);    
    void *y = sf_malloc(96);
    void *z = sf_malloc(128);
    memset(w, 1, 32);
    memset(x, 2, 64);
    memset(y, 3, 96);
    memset(z, 4, 128);
    cr_assert(freelist_head==(z-8+144));
    sf_free(w); //48
    sf_free(y); //112
    sf_free(x); //48+112+80 = 240
    cr_assert(freelist_head==(w-8));
    cr_assert(freelist_head->header.padding_size == 0);
    cr_assert(freelist_head->header.block_size << 4 == 240);
    cr_assert(freelist_head->next == (z-8+144));
    cr_assert(freelist_head->prev == NULL);
}

Test(sf_memsuite, Coalesce_both_block_2, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *w = sf_malloc(32);
    void *x = sf_malloc(64);    
    void *y = sf_malloc(96);
    void *z = sf_malloc(128);
    void *hi = sf_malloc(256);
    void *bye = sf_malloc(512);
    memset(w, 1, 32);
    memset(x, 2, 64);
    memset(y, 3, 96);
    memset(z, 4, 128);
    memset(hi, 5, 256);
    memset(bye, 6, 512);
    sf_free(y); //112
    sf_free(x); //112+80 = 192
    cr_assert(freelist_head==(x-8));
    cr_assert(freelist_head->header.padding_size == 0);
    cr_assert(freelist_head->header.block_size << 4 == 192);
    cr_assert(freelist_head->next == (bye-8+528));
    cr_assert(freelist_head->prev == NULL);
}