#pragma once
#include "RVCOS.h"
#include "double_queue.h"
#include <stdint.h>



typedef struct FreeListNode {

    struct FreeListNode *next;
    // struct FreeListNode *prev; //?

    char * begin; //char so that begin[4] is 4 bytes further than begin[0]
    // uint32_t * end; //redundant?
    TMemorySize size;

} FreeListNode;



typedef struct FreeList{

    FreeListNode *head;
    //freesize?
} FreeList;


typedef struct MemPool{
    char *base;
    TMemoryPoolID poolid;
    TMemorySize size;
    TMemorySize freesize;
    FreeList freeList;


} MemPool;

typedef struct MemoryHeader {
    TMemorySize chunksize;
    uint64_t chkvalue;
} MemoryHeader;

MemPool* POOL_get(TMemoryPoolID poolid);

#define RVCMemoryAllocate(size,pointer) RVCMemoryPoolAllocate(RVCOS_MEMORY_POOL_ID_SYSTEM, (size), (pointer))
#define RVCMemoryDeallocate(pointer) RVCMemoryPoolDeallocate(RVCOS_MEMORY_POOL_ID_SYSTEM, (pointer))
TStatus RVCMemoryPoolCreate(void *base, TMemorySize size, TMemoryPoolIDRef memoryref);
TStatus RVCMemoryPoolDelete(TMemoryPoolID memory);
TStatus RVCMemoryPoolQuery(TMemoryPoolID memory, TMemorySizeRef bytesleft);
TStatus RVCMemoryPoolAllocate(TMemoryPoolID memory, TMemorySize size, void **pointer);
TStatus RVCMemoryPoolDeallocate(TMemoryPoolID memory, void *pointer);

void print_address(uint64_t addr);


FreeListNode* allocateNewFreeListNode();
void MEMORY_init();
void FREELIST_initialize(FreeList *freelist, TMemorySize size, char* base);
void* FREELIST_reserve_first_fit(FreeList *freelist, TMemorySize size);
void FREELIST_free_and_coalesce(FreeList *freelist, char* base, TMemorySize amount_to_free);
void FREELISTNODE_delete(FreeListNode* deleteMe);
void FREELISTNODE_delete_recursive(FreeListNode* deleteMe);


      