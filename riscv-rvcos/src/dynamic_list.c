#include "dynamic_list.h"
#include "memory_manager.h"

void* toobig(){return NULL;}
void* cantfind(){return NULL;}

void* DLIST_get(DList* list, DLIST_ID_T id){
	if (id >= list->numItems){
		return toobig();
	}
	return list->array[id];
}
void DLIST_initialize(DList *dlist_to_init){
	dlist_to_init->array_allocated_length = 256;
	dlist_to_init->numItems = 0;
	RVCMemoryAllocate(dlist_to_init->array_allocated_length * sizeof(void *), &(dlist_to_init->array));
}


DList* DLIST_create(){
	DList* list;
	RVCMemoryAllocate(sizeof(DList), &list);
	list->array_allocated_length = 256;
	list->numItems = 0;
	RVCMemoryAllocate(list->array_allocated_length * sizeof(void *), &(list->array));
	return list;
}

void bigmistake(){while (1){}}
void hugemistake(){while (1){}}
void negativemistake(){while (1){}}

void reallocate(DList *list, int new_quantity){
	void** old_allocation = list->array; 
	int old_size = list->array_allocated_length;
	if (new_quantity <= old_size) { bigmistake(); } //instead of using an "assert" type of statement
	if (new_quantity >= ((uint32_t)((int32_t) -1))){ hugemistake(); }; //check that list isn't absurdly huge for some reason
	if (new_quantity <= 0){ negativemistake(); }

	RVCMemoryAllocate(new_quantity * sizeof(void *), &(list->array));
	//copy old into new	
	for (int i = 0; i < old_size; i++){
		list->array[i] = old_allocation[i];
	}
	//deallocate old list
	RVCMemoryDeallocate(old_allocation);
	list->array_allocated_length = new_quantity;
}


DLIST_ID_T DLIST_put(DList* list, void *item){
	if (list->numItems == list->array_allocated_length){
		reallocate(list, list->array_allocated_length * 2);
	}
	DLIST_ID_T id = list->numItems;
	list->numItems += 1;
	list->array[id] = item;
	return id;
}
