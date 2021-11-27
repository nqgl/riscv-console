#pragma once
#include "tcb.h"
#include "double_queue.h"
#define WAIT_TYPE_VIDEO 1
#define WAIT_TYPE_THREAD 0
#define WAIT_TYPE_MUTEX 2

typedef struct PriorityQueue PriorityQueue;
typedef struct Mutex Mutex;
typedef struct TCB TCB;

struct PriorityQueue{
    int size;
    DoubleQueue dq0;
    DoubleQueue dq1;
    DoubleQueue dq2;
    DoubleQueue dq3;
}; 



typedef struct VideoWriteData{
    const TTextCharacter *buffer;
    TMemorySize writesize;
}VideoWriteData;

void VIDEO_wait_initiate(const TTextCharacter *buffer, TMemorySize writesize);
void VIDEO_interrupt_write();
void SCHED_init();
void SCHED_remove(TCB *thread);
void SCHED_add(TCB *thread);
// TCB *SCHED_popnext();
void SCHED_runThread(TCB *thread);
void SCHED_schedule();
void SCHED_sleepRunning();

PriorityQueue* PQ_create();
DQNode *PQ_put(PriorityQueue* q, void* contents, TThreadPriority prio);
void* PQ_pop(PriorityQueue* q);
void *SCHED_PQ_pop(PriorityQueue *q);

void WAIT_set_wait(TCB* waiter);
TCB* WAIT_pop_waiter(TThreadID id, int waittype);
void WAIT_wake_up(TCB* waiter);
void WAIT_start_waiting();
void WAIT_stall_forever();
void PQ_delete(PriorityQueue *q);
// TCB* handle_timer_interrupt();