#pragma once
#include "RVCOS.h"
#include <stddef.h>
typedef struct DQNode DQNode;
typedef struct PriorityQueue PriorityQueue;

typedef struct DoubleQueue{
	DQNode *front;
	DQNode *back;

	PriorityQueue *pq;
	int size;
	//int maxsize?
} DoubleQueue;


typedef struct DQNode{
	struct DQNode *next;
	struct DQNode *prev;
	void* contents;
	struct DoubleQueue *parent_queue;
} DQNode;


typedef struct Mutex Mutex;
typedef struct TCB TCB;






DoubleQueue *make_DoubleQueue();
void delete_DoubleQueue(DoubleQueue *q);


void init_DoubleQueue(DoubleQueue* queue);
DQNode *DQNode_make(void* contents, DoubleQueue *queue);
void DQNode_delete(DQNode *node);
DQNode *dq_push_back(DoubleQueue* queue, void* object);
DQNode *dq_push_front(DoubleQueue* queue, void* object);
void* dq_pop_back(DoubleQueue* queue);
void* dq_pop_front(DoubleQueue* queue);

