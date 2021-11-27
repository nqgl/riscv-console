#include "tcb.h"
#include "RVCOS.h"
#include <stdint.h>
// #include <sys/types.h>
#include "dynamic_list.h"
#include <stdlib.h>
#include "memory_manager.h"



TCB mainTCB_not_pointer;
TCB* mainTCB = &mainTCB_not_pointer; //const?
TCB* idleTCB = NULL;

TCB* runningTCB = &mainTCB_not_pointer;
DList* TCBList = NULL;
    //but as we finish a2 and move to a3 we'll need a scalable way to track TCBs so there's the start of a 
    //binary tree struct below for future usage

    //for now, what we can do to make that transition easier down the line is to
    //not access by writing TCBArray[tcb_ID], but rather write it as getTCB(tcb_ID)

//make the new main thread TCB
extern volatile uint32_t *main_sp;


void TCB_init(){
}

TCB* makeNewMainTCB(uint32_t *gp){
    TCBList = DLIST_create();
    TCB* tcb = &mainTCB_not_pointer;
    tcb->tid = DLIST_put(TCBList, tcb);
    tcb->prio = RVCOS_THREAD_PRIORITY_NORMAL;
    tcb->state = RVCOS_THREAD_STATE_RUNNING;
    // tcb->entry
    tcb->param = NULL;
    tcb->memsize = 0;
    tcb->saved_sp = main_sp;
    tcb->retval = -1;
    // tcb->cursor_position = 0;
    tcb->stack_top = NULL;
    tcb->mutex_waiting_for = RVCOS_MUTEX_ID_INVALID;
    tcb->sleeps_in_node = NULL;
    return tcb;
}

TCB* TCB_allocate(){
    TCB* newTCB;
    RVCMemoryAllocate(sizeof(TCB), &newTCB);
    newTCB->owned_mutexes = make_DoubleQueue();
    newTCB->tcb_checking_constant = TCB_MAGIC_NUM;
    newTCB->tid = (TThreadID) DLIST_put(TCBList, newTCB);
    newTCB->sleeps_in_node = NULL;
    return newTCB;
}

//create a new TCB with appropriately filled in / nulled-out fields, and insert it into the
//datastructure used by the getTCB function
//for now they're already allocated in the TCBArray
//in the future it'll dynamically be allocating mem to the new TCB and
// to the TCB storage data structure
TCB* makeNewTCB(TThreadEntry entry, void *param, TMemorySize memsize, TThreadPriority prio, TThreadIDRef tid){
    TCB* tcb = TCB_allocate();
    tcb->prio = prio;
    tcb->state = RVCOS_THREAD_STATE_CREATED;
    // tcb->entry
    tcb->param = param;
    tcb->memsize = memsize;
    tcb->saved_sp = NULL; //when does SP of new thread get set up?
                    // I think we set this up when it's created
                    // will need to be coming back to that soon ish
    *tid = tcb->tid;
    tcb->retval = -1;
    // tcb->cursor_position = 0;
    RVCMemoryAllocate(memsize, &tcb->stack_top);
    tcb->entry = entry;
    tcb->mutex_waiting_for = RVCOS_MUTEX_ID_INVALID;    
    return tcb;
}


// if id is not a thread, return null
TCB* getTCB(TThreadID id){
    return DLIST_get(TCBList, (DLIST_ID_T) id);
}


