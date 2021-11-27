#include "memory_manager.h"
#include "RVCOS.h"
#include <stdint.h>
#include <stdlib.h>
#include "dynamic_list.h"

volatile uint32_t * freemin;

volatile uint32_t * max; 



#define rvcprint(s) \
            (RVCActuallyWriteText(s, sizeof(s)))
void print_address(uint64_t addr){
	char hexchars[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	int n;
	uint64_t num = addr;
	char array[] = "0x00000000";
	char* value = &array[2];
	for (int i = 0; i < 8; i++){
		n = num % 16;
		num = num / 16;
		value[7 - i] = hexchars[n];
	}
	// RVCActuallyWriteText("0x", 3);
	RVCActuallyWriteText(array, 10);
}


void print_out_freelist(FreeList *freelist){
	FreeListNode *node = freelist->head;
	rvcprint("\nfreelist:");
	while (node != NULL){
		rvcprint("\n\t-> addr:");
		print_address(node->begin);
		rvcprint(" \t size:");
		print_address(node->size);
		node = node->next;
	}

	rvcprint("\n\t-> NULL");

}


DList staticMemoryPoolsList;
DList *memoryPools = &staticMemoryPoolsList;
			//about[1 million]

// #define OS_POOL_SIZE ((TMemorySize) 1 << 20) //give os 1MB?
// char bigmemchunk[OS_POOL_SIZE];
extern uint8_t __stack_top[];
extern uint8_t _heapbase[];


char *bigmemchunk = _heapbase;
TMemorySize OS_POOL_SIZE =0;

void poolmistake(){
	while (1){
		
	}
}
#define STATIC_POOL_DLIST_MEM_SIZE ((TMemorySize) 256)
volatile char* static_data_pool[STATIC_POOL_DLIST_MEM_SIZE];
MemPool os_memory_pool;

void MEMORY_init(){
	OS_POOL_SIZE = (__stack_top - _heapbase - 64 * (4 << 10));

	staticMemoryPoolsList.array = &static_data_pool[0];
	staticMemoryPoolsList.array_allocated_length = STATIC_POOL_DLIST_MEM_SIZE;
	staticMemoryPoolsList.numItems = 0;
	os_memory_pool.base = bigmemchunk;
	os_memory_pool.size = OS_POOL_SIZE;
	os_memory_pool.freesize = OS_POOL_SIZE;
	FREELIST_initialize(&os_memory_pool.freeList, OS_POOL_SIZE, bigmemchunk);
	os_memory_pool.poolid = DLIST_put(memoryPools, &os_memory_pool);

	if (os_memory_pool.poolid != 0){
		poolmistake();
	}
}

MemPool* POOL_get(TMemoryPoolID poolid){
	return DLIST_get(memoryPools, poolid);
}


TStatus RVCMemoryPoolCreate(void *base, TMemorySize size, TMemoryPoolIDRef memoryref){
	//todo
	if (size < 128){
		return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
	}
	if(base == NULL || memoryref == NULL){
		return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
	}

	MemPool *newPool;
	RVCMemoryAllocate(sizeof(MemPool) ,&newPool);
	FREELIST_initialize(&newPool->freeList, size, base);
	newPool->size = size;
	newPool->freesize = size;
	newPool->base = base;
	newPool->poolid = DLIST_put(memoryPools, newPool);
	*memoryref = newPool->poolid;


	return RVCOS_STATUS_SUCCESS;
}



TStatus RVCMemoryPoolDelete(TMemoryPoolID memory){
	if (memory==RVCOS_MEMORY_POOL_ID_SYSTEM){
		return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
	}
	MemPool *pool = POOL_get(memory);
	if (pool == NULL){
		return RVCOS_STATUS_ERROR_INVALID_ID;
	}
	if (pool->size != pool->freesize){
		return RVCOS_STATUS_ERROR_INVALID_STATE;
	}
	memoryPools->array[memory] = NULL;
	FREELISTNODE_delete_recursive(pool->freeList.head);
	RVCMemoryDeallocate(pool);
	return RVCOS_STATUS_SUCCESS;

}

#define HEADER_SIZE ((TMemorySize) 16) //i believe sizeof(MemoryHeader) == 12 < 16 so this should be good
TStatus RVCMemoryPoolQuery(TMemoryPoolID memory, TMemorySizeRef bytesleft){
	MemPool *memPool = POOL_get(memory);
		if (memPool == NULL){
		return RVCOS_STATUS_ERROR_INVALID_ID;
	}
	if (bytesleft == NULL){
		return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
	}
	*bytesleft = memPool->freesize - HEADER_SIZE;
	return RVCOS_STATUS_SUCCESS;

}



TStatus RVCMemoryPoolAllocate(TMemoryPoolID memory, TMemorySize size, void **pointer){
	MemPool* memPool = POOL_get(memory);

	if (size == 0 || pointer == NULL){
		return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
	}
	if (memPool == NULL){
		return RVCOS_STATUS_ERROR_INVALID_ID;
	}
	// if (memory == RVCOS_MEMORY_POOL_ID_SYSTEM){
	// 	void* ptr_to_new_allocation = malloc(size);
	// 	*pointer = ptr_to_new_allocation;
	// 	return RVCOS_STATUS_SUCCESS;
	// }
	TMemorySize actual_size = size + HEADER_SIZE;
	if (actual_size < 64){
		actual_size = 64;
	}
	else if (actual_size % 16 != 0){
		actual_size = actual_size + 16 - actual_size % 16;
	}

	if (actual_size > memPool->freesize){
		return RVCOS_STATUS_ERROR_INSUFFICIENT_RESOURCES;
	}

	memPool->freesize -= actual_size;
	char* base = FREELIST_reserve_first_fit(&memPool->freeList, actual_size);
	MemoryHeader *header = (MemoryHeader*) base;
	
	header->chunksize = actual_size;
	header->chkvalue = ((uint64_t) 0xfe39ace);
	*pointer = &base[HEADER_SIZE];
	// RVCActuallyWriteText("\nalloc:", 6);
	//print_address(*pointer);
	//print_address(base);
	// print_out_freelist(&memPool->freeList);

	return RVCOS_STATUS_SUCCESS;	
}

void wrong_chkvalue_mistake(){while (1){}}


TStatus RVCMemoryPoolDeallocate(TMemoryPoolID memory, void *pointer){
	MemPool* memPool = POOL_get(memory);
	if (memPool == NULL){
		return RVCOS_STATUS_ERROR_INVALID_ID;
	}
	if (pointer == NULL){
		return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
	}
	// if (memory == RVCOS_MEMORY_POOL_ID_SYSTEM){
	// 	free(pointer);
	// 	return RVCOS_STATUS_SUCCESS;
	// }
	char* base = pointer;
	MemoryHeader *header = (MemoryHeader*) &base[-HEADER_SIZE];
	// RVCActuallyWriteText("\ndealloc:", 8);
	//print_address(base);
	//print_address(header);
	// print_out_freelist(&memPool->freeList);

	if (header->chkvalue != ((uint64_t) 0xfe39ace)){
		return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
	}
	memPool->freesize += header->chunksize;
	FREELIST_free_and_coalesce(&memPool->freeList, (void *) header, header->chunksize);
	return RVCOS_STATUS_SUCCESS;
}

FreeListNode initial_chunk_of_freelistnode[1024];
int freelistnode_index = 0;
FreeListNode *current_freelistnode_chunk = initial_chunk_of_freelistnode;

FreeListNode *deletedNodeStackTop = NULL;

FreeListNode* allocateNewFreeListNode(){
	if (deletedNodeStackTop == NULL){ //there are no deleted/recycled nodes to reuse
									// so we use statically allocated first 1024
									// after which dynamically allocated "slabs" of 1024
		if (freelistnode_index >= 1024){
			RVCMemoryAllocate(1024 * sizeof(FreeListNode), &current_freelistnode_chunk);
			freelistnode_index = 0;
		}	
		FreeListNode* newNode = &current_freelistnode_chunk[freelistnode_index];
		freelistnode_index += 1;
		return newNode;
	}
	else{ //there exist REUSED/deleted nodes    REUSE IS STRICTLY PREFERABLE TO RECYCLING
		//so we use that as the next node			^this isn't important it's a joke for myself
		FreeListNode *reused_node = deletedNodeStackTop;
		deletedNodeStackTop = reused_node->next;
		reused_node->next = NULL;
		return reused_node;
	}
}

void FREELISTNODE_delete(FreeListNode* deleteMe){
	deleteMe->begin = NULL;
	deleteMe->size = 0;
	if (deletedNodeStackTop == NULL){
		deleteMe->next = NULL;
		deletedNodeStackTop = deleteMe;
	}
	else{
		deleteMe->next = deletedNodeStackTop;
		deletedNodeStackTop = deleteMe;
	}

	//todo later
}


void FREELISTNODE_delete_recursive(FreeListNode* deleteMe){
	if (deleteMe->next != NULL){
		FREELISTNODE_delete_recursive(deleteMe->next);
	}
	FREELISTNODE_delete(deleteMe);
	//todo later
}






void FREELIST_initialize(FreeList *freelist, TMemorySize size, char* base){
	freelist->head = allocateNewFreeListNode();
	freelist->head = allocateNewFreeListNode();
	freelist->head->begin = base;
	freelist->head->size = size;
	freelist->head->next = NULL;
	freelist->head->begin = base;
	freelist->head->size = size;
	freelist->head->next = NULL;

}

void* FREELIST_reserve_first_fit(FreeList *freelist, TMemorySize size){
	FreeListNode *prevNode = NULL;
	FreeListNode *nextNode = freelist->head;
	int i = 0;
	while (nextNode->size < size && nextNode != NULL){
		prevNode = nextNode;
		nextNode = nextNode->next;

		i += 1;
		if (i > 1000){
			print_out_freelist(freelist);
		}
	}
	// rvcprint("out");
	if (nextNode == NULL){
		return NULL;
	}

	void* freespace = nextNode->begin;
	nextNode->begin += size;
	nextNode->size -= size;


	if (nextNode->size == 0){
		if (prevNode != NULL){
			prevNode->next = nextNode->next;
		} else{
			freelist->head = nextNode->next;
		}
		FREELISTNODE_delete(nextNode);
	}

	return freespace;
}



void FREELIST_free_and_coalesce(FreeList *freelist, char* base, TMemorySize amount_to_free){
	// node->begin = base;
	// node->size = amount_to_free;
	// rvcprint("\ncoalesce");
	// rvcprint("\n\tbase:");
	// print_address(base);
	// rvcprint("\n\tfreed:");
	// print_address(amount_to_free);

	int i = 0;
	FreeListNode *prevNode = NULL;
	FreeListNode *nextnode = freelist->head;
	while(nextnode->begin < base && nextnode != NULL){
		prevNode = nextnode;
		nextnode = nextnode->next;

	}
	FreeListNode *node;
	if (prevNode != NULL && prevNode->begin + prevNode->size == base){
		prevNode->size += amount_to_free;
		node = prevNode;
	}
	else{
		node = allocateNewFreeListNode();
		node->begin = base;
		node->size = amount_to_free;
		node->next = nextnode;
		if (prevNode != NULL){
			prevNode->next = node;
		}
		else{
			freelist->head = node;
		}
	}
	if (node != NULL && node->begin + node->size == nextnode->begin){
		node->size += nextnode->size;
		node->next = nextnode->next;
		FREELISTNODE_delete(nextnode);
	}
}