#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "hashmap.h"

typedef struct
{
    const char *key; // key is NULL if this is empty
    void* value; //void pointer for all types of data

}ht_entry;

struct ht
{
    ht_entry* entries; // table entries
    size_t capacity; // size of  entries array
    size_t length; // number of items in table
};

#define INITIAL_CAPACITY 16 // not zero

struct ht* ht_create(){
    struct ht* table = malloc(sizeof(struct ht));
    if(table == NULL)return NULL;

    // We initialize everything to 0
    ht_entry* entries = calloc(table->capacity,sizeof(ht_entry));
    if(entries == NULL){
        free(table);
        return NULL;
    }

    // Initialization
    table->capacity = INITIAL_CAPACITY;
    table->length = 0;
    table->entries = entries;

    return table;
}

struct ht* ht_free(struct ht* table){
    // Need to free all keys first and then the table entries 
    // if only entries the keys still allocated

    // for()
}