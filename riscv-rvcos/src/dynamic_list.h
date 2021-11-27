#include "RVCOS.h"
#include <stdint.h>
typedef struct DList{
	void** array;
	uint32_t array_allocated_length;
	uint32_t numItems;
} DList;

typedef uint32_t DLIST_ID_T;

DList* DLIST_create();
void DLIST_initialize(DList *dlist_to_init);

void* DLIST_get(DList* list, DLIST_ID_T id);
DLIST_ID_T DLIST_put(DList* list, void *item);

void reallocate(DList *list, int new_quantity);

//someday maybe a delete method

