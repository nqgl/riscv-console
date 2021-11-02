typedef int byte_size_type;
						//malloc analogue
typedef void* (*AllocFP)(byte_size_type); //declaring a type AllocFP, a function pointer with the same signature as malloc

					//free analogue
typedef void (*FreeFP)(void*); //decl type FreeFP = "free" style funcion pointer

AllocFP allocate = NULL;
FreeFP deallocate = NULL;

void set_allocator_function(AllocFP alloc_f){
	allocate = alloc_f;
}

void set_free_function(AllocFP alloc_f){
	allocate = alloc_f;
}


typedef struct DQNode{
	struct DQNode *next, *prev;
	void* contents;

} DQNode; 



struct DQueue{
	DQNode *front, *back;


	int size;
	//int maxsize?
} DQueue, *DQueueref;

DQueue *make_DQueue(){
	DQueue* queue = allocate(sizeof(DQueue));
	queue.front = NULL;
	queue.back = NULL;
	queue.size = 0;
	return queue;
}

DQNode *make_DQNode(void* contents){
	DQNode *node = allocate(sizeof(DQNode));
	node->contents = contents;
	node.next = NULL;
	node.prev = NULL;
	return node;
}


DQNode *make_DQNode(void* contents){
	DQNode *node = allocate(sizeof(DQNode));
	node->contents = contents;
	node.next = NULL;
	node.prev = NULL;
	return node;
}



dq_push_back(DQueue* queue, void* object){
	DQNode* newnode = make_DQNode(object);
	if (queue.size == 0){
		queue.front = newnode;
		queue.back = newnode;
	} else {
		newnode.next = queue.back;
		queue.back.prev = -;
		queue.back = newnode;
	}
}

dq_push_front(DQueue* queue, void* object){
	DQNode* newnode = make_DQNode(object);
	if (queue.size == 0){
		queue.front = newnode;
		queue.back = newnode;
	} else {
		newnode.prev = queue.front;
		queue.front.next = newnode;
		queue.front = newnode;
	}
}
dq_pop_back(DQueue* queue)
dq_pop_front(DQueue* queue)
dq_back(DQueue* queue)
{
	return queue.back.;
}
dq_front(DQueue* queue)
{
	return queue.front.;
}


