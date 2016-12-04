#ifndef LOTT_H
#define LOTT_H


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
 
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <semaphore.h>

#define DATA_DIR "mock"


#define HELP do{ \
                printf("%s\n", "Lord of the Threads");\
                printf("%s\n", "bin/lott N QUERY [M]");\
                printf("%s\n", "N - Part specification: 1, 2, 3, 4, 5 are valid choices.");\
                printf("%s\n", "QUERY - The calculation the program is to execute: A, B, C, D or E");\
                printf("%s\n", "M - Number of threads for parts that take a specified amount");\
            }while(0)

#define FOREACH_PART(PART) \
    PART(PART1)            \
    PART(PART2)            \
    PART(PART3)            \
    PART(PART4)            \
    PART(PART5)

#define FOREACH_QUERY(QUERY) \
    QUERY(A)               \
    QUERY(B)               \
    QUERY(C)               \
    QUERY(D)               \
    QUERY(E)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum QUERY_ENUM { FOREACH_QUERY(GENERATE_ENUM) };
typedef enum QUERY_ENUM Query;
static const char *QUERY_STRINGS[] = {FOREACH_QUERY(GENERATE_STRING)};

enum PART_ENUM { FOREACH_PART(GENERATE_ENUM) };
typedef enum PART_ENUM Part;
static const char *PART_STRINGS[] = {FOREACH_PART(GENERATE_STRING)};

Query current_query;
Part current_part;

int part1();
int part2(size_t);
int part3(size_t);
int part4(size_t);
int part5(size_t);

struct Stats {
  double avgduration;
  double avgusercountperyear;             
  char maxCC[2];  
  int maxCCcount;          
  char* filename;
  struct Stats* next;
};
typedef struct Stats Stats;


Stats* statshead;
Stats* firststatshead;

struct yearstruct
{
    int count;
    int year;
    struct yearstruct *next;
};
typedef struct yearstruct yearstruct;

struct countrystruct
{
    int count;
    char ccode[2];
    struct countrystruct *next;
};
typedef struct countrystruct countrystruct;

int nfiles();

void addStatToList(Stats* add);

void printstats();

void werrorchut(void*);

void pushyeartolist(yearstruct **head, yearstruct *item);

void freeyears(yearstruct *head);

void freecountry(countrystruct *head);

void freestats(Stats *head);

double calculateDistintYears(yearstruct *head);


Stats* createStat();

yearstruct* createyear(int value);

yearstruct* findyear(yearstruct *head, int value);

countrystruct* createcountry(char *value);

countrystruct* findcountry(countrystruct *head, char* value);

void pushcountrytolist(countrystruct **head, countrystruct *item);

countrystruct* findmaxccodes(countrystruct *head);

countrystruct* createcountryforreduce(char *value, int count);

void* helpmap3(void* dir);

void* helpmap(void* dir);

#endif /* LOTT_H */