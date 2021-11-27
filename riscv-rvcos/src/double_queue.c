#include "double_queue.h"
#include "memory_manager.h"
#include "scheduler.h"
#include "RVCOS.h"




void init_DoubleQueue(DoubleQueue* queue){
	queue->front = NULL;
	queue->back = NULL;
	queue->size = 0;

	queue->pq = NULL;
}


DoubleQueue *make_DoubleQueue(){
	DoubleQueue* queue; 
	RVCMemoryAllocate(sizeof(DoubleQueue), &queue);
	init_DoubleQueue(queue);
	return queue;
}

void delete_DoubleQueue(DoubleQueue *q){
	DQNode *deleteMe;
	while (q->size > 0){
		deleteMe = dq_pop_back(q);
		DQNode_delete(deleteMe);
	}
	RVCMemoryDeallocate(q);
}


DQNode *DQNode_make(void* contents, DoubleQueue *parent){
	DQNode *node;
	RVCMemoryAllocate(sizeof(DQNode), &node);
	node->contents = contents;
	node->next = NULL;
	node->prev = NULL;
	node->parent_queue = parent;
	return node;
}


void DQNode_delete(DQNode *node){
	//todo

	if (node == node->parent_queue->front){
		node->parent_queue->front = node->prev;
	}
	if (node == node->parent_queue->back){
		node->parent_queue->back = node->next;
	}
	node->parent_queue->size -= 1;
	if (node->parent_queue->pq != NULL){
		node->parent_queue->pq->size -= 1;
	}
	node->next->prev = node->prev;
	node->prev->next = node->next;
	node->next = NULL;
	node->prev = NULL;
	RVCMemoryDeallocate(node);
}



DQNode* dq_push_back(DoubleQueue* queue, void* object){
	DQNode* newnode = DQNode_make(object, queue);
	if (queue->size == 0){
		queue->front = newnode;
		queue->back = newnode;
	} else {
		newnode->next = queue->back;
		queue->back->prev = newnode;
		queue->back = newnode;
	}
	queue->size += 1;
	return newnode;
}

DQNode* dq_push_front(DoubleQueue* queue, void* object){
	DQNode* newnode = DQNode_make(object, queue);
	if (queue->size == 0){
		queue->front = newnode;
		queue->back = newnode;
	} else {
		newnode->prev = queue->front;
		queue->front->next = newnode;
		queue->front = newnode;
	}
	queue->size += 1;
	return newnode;
}


void* dq_pop_back(DoubleQueue* queue){
	if (queue->back == NULL) {
		return NULL;
		//really this ought to throw or at least print an error
	}
	queue->size -= 1;
	DQNode *pop = queue->back;
	if (pop->next == NULL){
		queue->back = NULL;
		queue->front = NULL;
	}
	else {
		queue->back = pop->next;
		if(queue->back != NULL){
			queue->back->prev = NULL;
		}
		
	}
		// DQNode_delete(pop);
	return pop->contents;
}
void* dq_pop_front(DoubleQueue* queue){
	if (queue == NULL){
		// RVCWriteText("NULL queue pa#sed to dq_pop_front", 33);
	}
	if (queue->front == NULL) {
		return NULL;
		//really this ought to throw or at least print an error
	}
	if (queue->back == NULL){
		// RVCWriteText("qfront nonnull, back null?!\n", 28);
	}
	queue->size -= 1;
	DQNode *pop = queue->front;
	// RVCWriteText("t0\n", 3);
	if (pop->prev == NULL){ //error for some reason
		// RVCWriteText("t1\n", 3);
		queue->back = NULL;
		queue->front = NULL;
		// RVCWriteText("t2\n", 3);
	}
	else {
		// RVCWriteText("t3\n", 3);
		queue->front = pop->prev;
		if(queue->front != NULL){
			queue->front->next = NULL;
		}
		
		// RVCWriteText("t4\n", 3);
	}
	// RVCWriteText("t5\n", 3);
	// RVCWriteText("t6\n", 3);
	
	// DQNode_delete(pop);
	return pop->contents;
	
}

void* dq_back(DoubleQueue* queue)
{
	return queue->back->contents;
}

void* dq_front(DoubleQueue* queue)
{
	return queue->front->contents;
}

