#include "mutex.h"
#include "dynamic_list.h"
#include "memory_manager.h"
#include "double_queue.h"
#include "scheduler.h"
#include "tcb.h"


#define print_debug(s) \
            (RVCActuallyWriteText(s, sizeof(s)))




DList* mutex_list;


void MUTEX_init(){
    mutex_list = DLIST_create();
    
}
Mutex *MUTEX_create(){
    Mutex* newtex; 
    RVCMemoryAllocate(sizeof(Mutex), &newtex);
    newtex->id = DLIST_put(mutex_list, newtex);
    newtex->waiters = PQ_create();
    newtex->owner = RVCOS_THREAD_ID_INVALID; //mutex starts
    return newtex;
}

void MUTEX_released_update(Mutex *mtx){
    print_debug("going to unlock");
    if (mtx->waiters->size == 0){
        print_debug("mutex became unlocked");
        mtx->owner = RVCOS_THREAD_ID_INVALID; //puts mutex into "unlocked" state
    } else {
        TCB* next_owner = PQ_pop(mtx->waiters);

        DQNode_delete(next_owner->sleeps_in_node); //removes it from the sleep queue
        next_owner->mutex_waiting_for = RVCOS_MUTEX_ID_INVALID;
        MUTEX_take(mtx, next_owner->tid);
        SCHED_add(next_owner);
    }
}

Mutex* getMutex(TMutexID mutex){
    return DLIST_get(mutex_list, mutex);
}
void MUTEX_delete(Mutex* mtx){
    PQ_delete(mtx->waiters);
    RVCMemoryDeallocate(mtx);
}

    
int MUTEX_take(Mutex* mtx, TThreadID thread){
    mtx->owner = thread;
    mtx->tcb_owner_node = dq_push_front(getTCB(thread)->owned_mutexes, mtx);
}


int MUTEX_unlocked(Mutex* mtx){
    return mtx->owner  == RVCOS_THREAD_ID_INVALID;
}

extern TTick tickTime;

int MUTEX_acquire(Mutex* mtx, TCB* thread, TTick timeout){
    if (MUTEX_unlocked(mtx)){
        print_debug("already unlocked");
        MUTEX_take(mtx, thread->tid);
        return 1;
    }
    else{
        if (timeout == RVCOS_TIMEOUT_IMMEDIATE){
            return 0; //false?
        }
        else{
            print_debug("something got queued");
            //add it to queue here and as a sleeper
            thread->mutex_waiting_for = mtx->id;
            thread->mutex_queue_node = PQ_put(mtx->waiters, thread, thread->prio);
            thread->waittype = WAIT_TYPE_MUTEX;
            if (timeout != RVCOS_TIMEOUT_INFINITE){
                thread->waits_until = tickTime + timeout;
            }
            else {
                thread->waits_until = RVCOS_TIMEOUT_INFINITE;
            }
            WAIT_start_waiting();

            //we will need to update the code for sleep to check if it is a mutex sleeper and deque from both queues upon waking
        }
    }
}

void MUTEX_release(Mutex* mtx){
    DQNode_delete(mtx->tcb_owner_node);
    MUTEX_released_update(mtx);
}

