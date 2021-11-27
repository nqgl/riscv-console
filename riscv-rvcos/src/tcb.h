#pragma once
#include "RVCOS.h"
#include <stdint.h>
#include "double_queue.h"
#define TCB_MAGIC_NUM ((uint64_t) 0x1432627384abbcde)

typedef struct DQNode DQNode; 
typedef struct TCB TCB;
struct TCB // I removed the volatile keyword. Hopefully that's good
{                              // I'm not sure that tcb needs to be volatile
                                //but if they do, when we instantiate a TCB is when we would declare volatile
                                //eg: "volatile TCB volatile_tcb_variable_name;"
    uint64_t tcb_checking_constant;
    TThreadEntry entry;
    void *param;                
    TMemorySize memsize;
    TThreadPriority prio;
    // uint32_t *sp; //I can't figure out if this should be a pointer 
    uint32_t volatile *saved_sp;
    TThreadID tid; //also unsure about this one
    TThreadState state;
    TThreadReturn retval;
    // int32_t cursor_position;
    // int firstRun; //boolean
    TThreadID waits_for; //tid of the thread waiting for termination of
    TTick waits_until;
    int waitsuccess;
    TThreadReturnRef return_value_storage_pointer;
    TMutexID mutex_waiting_for; //for when waiting to get a mutex
    DQNode *mutex_queue_node;
    int waittype;
    DQNode *sleeps_in_node;
    uint8_t *stack_top;
    DoubleQueue *owned_mutexes;
};



TCB* makeNewMainTCB(uint32_t *gp);
TCB* makeNewTCB(TThreadEntry entry, void *param, TMemorySize memsize, TThreadPriority prio, TThreadIDRef tid);
TCB* getTCB(TThreadID id);
