#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "helper.h"
#include "sfmm.h"

#include <stdio.h>

// Colors
#ifdef COLOR
    #define KNRM  "\x1B[0m"
    #define KRED  "\x1B[1;31m"
    #define KGRN  "\x1B[1;32m"
    #define KYEL  "\x1B[1;33m"
    #define KBLU  "\x1B[1;34m"
    #define KMAG  "\x1B[1;35m"
    #define KCYN  "\x1B[1;36m"
    #define KWHT  "\x1B[1;37m"
    #define KBWN  "\x1B[0;33m"
#else
    /* Color was either not defined or Terminal did not support */
    #define KNRM
    #define KRED
    #define KGRN
    #define KYEL
    #define KBLU
    #define KMAG
    #define KCYN
    #define KWHT
    #define KBWN
#endif

#ifdef DEBUG
    #define debug(S, ...)   fprintf(stdout, KMAG "DEBUG: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define error(S, ...)   fprintf(stderr, KRED "ERROR: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define warn(S, ...)    fprintf(stderr, KYEL "WARN: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define info(S, ...)    fprintf(stdout, KBLU "INFO: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define success(S, ...) fprintf(stdout, KGRN "SUCCESS: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
    #define debug(S, ...)
    #define error(S, ...)   fprintf(stderr, KRED "ERROR: " KNRM S, ##__VA_ARGS__)
    #define warn(S, ...)    fprintf(stderr, KYEL "WARN: " KNRM S, ##__VA_ARGS__)
    #define info(S, ...)    fprintf(stdout, KBLU "INFO: " KNRM S, ##__VA_ARGS__)
    #define success(S, ...) fprintf(stdout, KGRN "SUCCESS: " KNRM S, ##__VA_ARGS__)
#endif

// Define 20 megabytes as the max heap size
#define MAX_HEAP_SIZE (20 * (1 << 20))
#define VALUE1_VALUE 320
#define VALUE2_VALUE 0xDEADBEEFF00D

#define press_to_cont() do { \
    printf("Press Enter to Continue"); \
    while(getchar() != '\n'); \
    printf("\n"); \
} while(0)

#define null_check(ptr, size) do { \
    if ((ptr) == NULL) { \
        error("Failed to allocate %lu byte(s) for an integer using sf_malloc.\n", (size)); \
        error("Aborting...\n"); \
        assert(false); \
    } else { \
        success("sf_malloc returned a non-null address: %p\n", (ptr)); \
    } \
} while(0)

#define payload_check(ptr) do { \
    if ((unsigned long)(ptr) % 16 != 0) { \
        warn("Returned payload address is not divisble by a quadword. %p %% 16 = %lu\n", (ptr), (unsigned long)(ptr) % 16); \
    } \
} while(0)

#define check_prim_contents(actual_value, expected_value, fmt_spec, name) do { \
    if (*(actual_value) != (expected_value)) { \
        error("Expected " name " to be " fmt_spec " but got " fmt_spec "\n", (expected_value), *(actual_value)); \
        error("Aborting...\n"); \
        assert(false); \
    } else { \
        success(name " retained the value of " fmt_spec " after assignment\n", (expected_value)); \
    } \
} while(0)

int main(int argc, char *argv[]) {
    // Initialize the custom allocator
    sf_mem_init(MAX_HEAP_SIZE);

      //int *value26 = sf_malloc(8);
     // int *value28 = sf_malloc(16);
     // sf_free(value28);
     // int *value28 = sf_malloc(16);
     // int *value30 = sf_malloc(16);
     // *value26 = 26;
     // *value28 = 28;
     // sf_free(value28);
     // int *value32 = sf_realloc(value26,64);
     // check_prim_contents(value32, 26, "%d", "value26");
     // sf_varprint(value32);
     
     // printblocks();
     //info inf;
     //sf_info(&inf);
     //printf("***inf malloc%zu\n",inf.allocations);
     //printf("***inf coalesce%zu\n",inf.coalesce);
     //printf("***inf free %zu\n",inf.frees);
     //printf("***inf internal%zu\n",inf.internal);
     //printf("***inf external%zu\n",inf.external);
    // int *value30 = sf_malloc(16);
    // //sf_free(value28);
    //*value26 = 26;
    // printblocks();
    // int *value32 = sf_realloc(value26,64);
    // check_prim_contents(value32, 26, "%d", "value32");
    // printblocks();
    //sf_varprint(value28);
    //sf_varprint(value30);
    // sf_varprint(value32);
   

    // Tell the user about the fields
    info("Initialized heap with %dmb of heap space.\n", MAX_HEAP_SIZE >> 20);
    press_to_cont();

     

    // Print out title for first test
    printf("=== Test1: Allocation test ===\n");
    // Test #1: Allocate an integer
    int *value1 = sf_malloc(sizeof(int));
    int *value9 = sf_malloc(sizeof(int));
    int *value3= sf_malloc(sizeof(int));
    int *value4 = sf_malloc(sizeof(int));
    int *value5 = sf_malloc(sizeof(int));
    int *value6 = sf_malloc(sizeof(int));
    int *value7 = sf_malloc(sizeof(int));
    sf_varprint(value1);
    sf_varprint(value9);
    sf_varprint(value3);
    sf_varprint(value4);
    sf_varprint(value5);
    sf_varprint(value6);
    sf_varprint(value7);
    *value7 = VALUE1_VALUE;
    //sf_free(value1);
    //sf_free(value3);
    //sf_free(value5);
    sf_free(value7);
    //sf_free(value6);
    printblocks();

    null_check(value1, sizeof(int));
    payload_check(value1);
    // Print out the allocator block
    //sf_varprint(value1);
    press_to_cont();

    // Now assign a value
    printf("=== Test2: Assignment test ===\n");
    info("Attempting to assign value1 = %d\n", VALUE1_VALUE);
    // Assign the value
    *value1 = VALUE1_VALUE;
    // Now check its value
    check_prim_contents(value1, VALUE1_VALUE, "%d", "value1");
    press_to_cont();

    printf("=== Test3: Allocate a second variable ===\n");
    info("Attempting to assign value2 = %ld\n", VALUE2_VALUE);
    long *value2 = sf_malloc(sizeof(long));
    null_check(value2, sizeof(long));
    payload_check(value2);
    sf_varprint(value2);
    // Assign a value
    *value2 = VALUE2_VALUE;
    // Check value
    check_prim_contents(value2, VALUE2_VALUE, "%ld", "value2");
    press_to_cont();

    printf("=== Test4: does value1 still equal %d ===\n", VALUE1_VALUE);
    check_prim_contents(value1, VALUE1_VALUE, "%d", "value1");
    press_to_cont();

    // Snapshot the freelist
    printf("=== Test5: Perform a snapshot ===\n");
    sf_snapshot(true);
    press_to_cont();

    // Free a variable
    printf("=== Test6: Free a block and snapshot ===\n");
    info("Freeing value1...\n");
    sf_free(value1);
    sf_snapshot(true);
    press_to_cont();

    // Allocate more memory
    printf("=== Test7: 8192 byte allocation ===\n");
    void *memory = sf_malloc(8192);
    sf_varprint(memory);
    sf_free(memory);
    press_to_cont();

    sf_mem_fini();

    return EXIT_SUCCESS;
}
