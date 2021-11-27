#pragma once
#include "RVCOS.h"
#include "double_queue.h"
#include "tcb.h"
#include "scheduler.h"
typedef struct PriorityQueue PriorityQueue;
typedef struct DoubleQueue DoubleQueue;
typedef struct DQNode DQNode; 
typedef struct Mutex Mutex;
typedef struct TCB TCB;


struct Mutex{
    volatile TThreadID owner; //locked if owner == owner
                    //unlock if owner == invalid
    PriorityQueue* waiters;
    TMutexID id;
    DQNode* tcb_owner_node;
};





void MUTEX_released_update();

Mutex* MUTEX_create();
Mutex* getMutex(TMutexID mutex);
void MUTEX_delete(Mutex* mtx);
int MUTEX_take(Mutex* mtx, TThreadID thread); 
int MUTEX_acquire(Mutex* mtx, TCB* thread, TTick timeout);
void MUTEX_release(Mutex* mtx);
void MUTEX_init();
int MUTEX_unlocked(Mutex* mtx);