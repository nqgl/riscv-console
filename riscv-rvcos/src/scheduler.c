

#include "RVCOS.h"
#include "scheduler.h"
#include "tcb.h"
#include "double_queue.h"
#include "memory_manager.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include "mutex.h"
#define print_debug(s) \
            (RVCActuallyWriteText(s, sizeof(s)))






static PriorityQueue* scheduler_queue = NULL;
extern TCB* runningTCB;
extern volatile uint32_t* global;
uint32_t ContextSwitch(volatile uint32_t** from, volatile uint32_t* to);
uint32_t call_on_other_gp(void*, TThreadEntry, uint32_t*);
int initialized = 0;

DoubleQueue* sleepers;
DoubleQueue* waiters;

DoubleQueue* video_write_queue;
void SCHED_init(){
    INTERRUPT_ENABLE_REG = INTERRUPT_ENABLE_REG | 0x2;
    scheduler_queue = PQ_create();
    scheduler_queue->size = 0;
    initialized = 1;
    sleepers = make_DoubleQueue();
    waiters = make_DoubleQueue();
    video_write_queue = make_DoubleQueue();
}

//todo but not for a while
//needs to be called from terminate?
    //related: should we have a place to put dead threads?


void VIDEO_wait_initiate(const TTextCharacter *buffer, TMemorySize writesize){
    VideoWriteData *writedata;
    RVCMemoryAllocate(sizeof(VideoWriteData),&writedata);
    writedata->buffer = buffer;
    writedata->writesize = writesize;
    dq_push_back(video_write_queue, writedata);
    runningTCB->waits_for = RVCOS_THREAD_ID_INVALID;
    runningTCB->waits_until = RVCOS_TIMEOUT_INFINITE;
    runningTCB->state = RVCOS_THREAD_STATE_WAITING;
    runningTCB->waittype = WAIT_TYPE_VIDEO;
    WAIT_start_waiting();
}

void VIDEO_interrupt_write(){
    //write the video write queue first
    VideoWriteData* writedata;
    while (video_write_queue->size > 0){
        writedata = dq_pop_front(video_write_queue);
        RVCActuallyWriteText(writedata->buffer, writedata->writesize);
    }

    //then wake threads

    TCB* video_waiting_thread = WAIT_pop_waiter(RVCOS_THREAD_ID_INVALID, WAIT_TYPE_VIDEO);
    while (video_waiting_thread != NULL){
        video_waiting_thread->waits_for = RVCOS_THREAD_ID_INVALID;
        video_waiting_thread->state = RVCOS_THREAD_STATE_READY;
        SCHED_add(video_waiting_thread);
        video_waiting_thread = WAIT_pop_waiter(RVCOS_THREAD_ID_INVALID, WAIT_TYPE_VIDEO);
    }

}




TCB* WAIT_pop_waiter(TThreadID id, int waittype){
    int numwaiting = waiters->size;
    for (int i = 0; i < numwaiting; i++){
        TCB* popped = dq_pop_back(waiters);
        if ((popped->waits_for == id)){
            // RVCActuallyWriteText("wait12345678", 4 + waittype);
            // RVCActuallyWriteText("wpop12345678", 4 + popped->waittype);
            if (popped->waittype == waittype){
                return popped;
            }
            else{
            }
        }
        dq_push_front(waiters, popped);
    }
    return NULL;
}


void SCHED_add(TCB *thread){
    if (thread->tid == 1){
        // RVCActuallyWriteText("schc12345678", 4 + scheduler_queue->size);
    }
    if (thread != NULL){
        PQ_put(scheduler_queue, thread, thread->prio);
        // RVCWriteText("schc12345678", 4 + scheduler->size);
    }
}

// TCB *SCHED_popnext(){
//     TCB *popped = PQ_pop(scheduler);

// }


//yes, this SHOULD trigger another thred sched
    //(in the case that the thread removing is already running)
void SCHED_remove(TCB *thread){
    if (runningTCB == thread){
        thread->state = RVCOS_THREAD_STATE_DEAD;
        SCHED_runThread(SCHED_PQ_pop(scheduler_queue));
    }
    DoubleQueue* q;
    for (int i = 0; i <= 3; i++){
        if (i == 0){
            q = &scheduler_queue->dq0;
        }
        if (i == 1){
            q = &scheduler_queue->dq1;
        }
        if (i == 2){
            q = &scheduler_queue->dq2;
        }
        if (i == 3){
            q = &scheduler_queue->dq3;
        }
        
        DQNode* node = q->front;
        while (node != NULL && ((TCB*) node->contents)->tid != thread->tid){
            node = node->prev;
        }
        if (node != NULL){
            DQNode_delete(node);
        }
    }
}
int unimportant = 0;
void called_by_idle(){
    while(1){

    }
}

void WAIT_start_waiting(){
    // print_debug("wait_start_waiting called");
    if (initialized){
        if (scheduler_queue->size > 0){
        }
        else{
            RVCActuallyWriteText("empty sched in startwaiting", 28);
        }
        if (runningTCB->tid == 1){
            called_by_idle();
        }
        runningTCB->sleeps_in_node = dq_push_front(waiters, runningTCB);
        runningTCB->state = RVCOS_THREAD_STATE_WAITING;
        SCHED_runThread(SCHED_PQ_pop(scheduler_queue));
    }
    else{
        RVCActuallyWriteText("no init in startwaaaaaait", 23);
    }
}

void WAIT_stall_forever(){
    SCHED_runThread(SCHED_PQ_pop(scheduler_queue));
}
void SCHED_sleepRunning(){
    if (initialized){
        if (scheduler_queue->size > 0){
            runningTCB->sleeps_in_node = dq_push_front(sleepers, runningTCB);
            SCHED_runThread(SCHED_PQ_pop(scheduler_queue));
        }
        else{
            RVCWriteText("empty sched in sleeprunnning", 28);
        }
    }
    else{
        RVCWriteText("no init in sleeprunnning", 23);
    }
}


extern volatile TTick tickTime;
void SCHED_schedule(){
    if (initialized){
        char nums[10] = {'0','1','2','3','4','5','6','7','8','9'};

        // RVCWriteText("sch012345678", 4 + scheduler->size);
        // RVCWriteText((nums+scheduler->size), 1);
        int numsleep = sleepers->size;
        TCB* popped;
        for (int i = 0; i < numsleep; i++){
            popped = dq_pop_back(sleepers);
            if (popped->waits_until < tickTime){
                popped->state = RVCOS_THREAD_STATE_READY;
                SCHED_add(popped);
            } else{
                dq_push_front(sleepers, popped);
            }
        }        
        int numwaiting = waiters->size;
        for (int i = 0; i < numwaiting; i++){
            popped = dq_pop_back(waiters);
            if (popped->waits_until < tickTime && popped->waits_until != RVCOS_TIMEOUT_INFINITE){
                popped->waits_for = RVCOS_THREAD_ID_INVALID;
                popped->state = RVCOS_THREAD_STATE_READY;

                SCHED_add(popped);
                if (popped->mutex_waiting_for != RVCOS_MUTEX_ID_INVALID){
                    // Mutex* mtx = getMutex(popped->mutex_waiting_for);
                    DQNode_delete(popped->mutex_queue_node);
                    popped->mutex_queue_node = NULL;
                    popped->mutex_waiting_for = RVCOS_MUTEX_ID_INVALID;
                }
            } else{
                dq_push_front(waiters, popped);
            }
        }
        if (runningTCB != NULL){
            runningTCB->state = RVCOS_THREAD_STATE_READY;
            SCHED_add(runningTCB);
        } else{
            RVCWriteText("BAD? NULL runningtcb\n", 17);            
        }
        if (scheduler_queue->size > 0){

            popped = SCHED_PQ_pop(scheduler_queue);
            SCHED_runThread(popped);
            if (popped != NULL){
            }else{
                RVCWriteText("BAD: NULLing in sched\n", 17);            
            }
        }
        else{
            // RVCWriteText("BAD: nothing in sched\n", 17);
        }
    }
}



// void runNewThread(TCB *thread){
//     TCB* switch_from = runningTCB;
//     runningTCB = thread;
//     thread->firstRun = 0;
//     SCHED_add(switch_from);
//     ContextSwitch(&switch_from->saved_sp, thread->saved_sp);

// }

void SCHED_runThread(TCB *thread){
    // if (thread->firstRun){
    //     runNewThread(thread);
    //     return;
    // }
    if(thread == NULL){
        RVCActuallyWriteText("null thread ran?", 16);
        return;
    }
    if(thread != runningTCB){
        TCB* switch_from = runningTCB;
        runningTCB = thread;
        thread->state = RVCOS_THREAD_STATE_RUNNING;
        if (switch_from == NULL){
            RVCWriteText("sfnull\n", 7);
            // ContextSwitch(&saved_sp, thread->saved_sp);
        }
        else{
            // RVCActuallyWriteText("from0123456789", 5 + switch_from->tid);
            // RVCActuallyWriteText("to0123456789", 3 + thread->tid);
            
            if (switch_from->state == RVCOS_THREAD_STATE_RUNNING){
                switch_from->state = RVCOS_THREAD_STATE_READY;
            }
            ContextSwitch(&switch_from->saved_sp, thread->saved_sp);
        }
    }
    else{
        runningTCB->state = RVCOS_THREAD_STATE_RUNNING;
    }
}
extern TCB* idleTCB;
void* SCHED_PQ_pop(PriorityQueue *q){
    void* threadpopped = PQ_pop(q);
    if (threadpopped == NULL){
        RVCActuallyWriteText("\nidle faked\n", 12);
        return idleTCB;
    }
    return threadpopped;
}





PriorityQueue* PQ_create(){
    PriorityQueue* pq;
    RVCMemoryAllocate(sizeof(PriorityQueue), &pq);
    // RVCWriteText("pq is initialized\n", 17);
    init_DoubleQueue(&pq->dq0);
    init_DoubleQueue(&pq->dq1);
    init_DoubleQueue(&pq->dq2);
    init_DoubleQueue(&pq->dq3);
    pq->dq0.pq = pq;
    pq->dq1.pq = pq;
    pq->dq2.pq = pq;
    pq->dq3.pq = pq;
    pq->size = 0;
    return pq;
}

void PQ_delete(PriorityQueue *q){
//todo    
}


DQNode *PQ_put(PriorityQueue* q, void* contents, TThreadPriority prio){
    // RVCWriteText("\nput1234567", 4+((TCB*)contents)->tid);
    DQNode *new_dq_node = NULL;
    if (prio == 0){
        new_dq_node = dq_push_back(&q->dq0, contents);
    }
    if (prio == 1){
        new_dq_node = dq_push_back(&q->dq1, contents);
    }
    if (prio == 2){
        new_dq_node = dq_push_back(&q->dq2, contents);
    }
    if (prio == 3){
        new_dq_node = dq_push_back(&q->dq3, contents);
    }
    q->size += 1;

    return new_dq_node;
}

void* PQ_pop(PriorityQueue* q){

    if (q == NULL){
        // RVCWriteText("PQ was null", 11);
    }
    if (q->size == 0){
        return NULL;
    }
    q->size -= 1;
    char nums[10] = {'0','1','2','3','4','5','6','7','8','9'};
    // RVCWriteText("pop", 3);
    void* retval = NULL;
    if (q->dq3.size > 0){
        retval = dq_pop_front(&q->dq3);
    }
    else if (q->dq2.size > 0){
        retval = dq_pop_front(&q->dq2);
    }
    else if (q->dq1.size > 0){
        retval = dq_pop_front(&q->dq1);
    }
    else if (q->dq0.size > 0){
        retval = dq_pop_front(&q->dq0);
    }
    TCB* possible_tcb = retval;
    // if (possible_tcb->tcb_checking_constant == TCB_MAGIC_NUM){
    //     if (possible_tcb->tid == 1){ //check if idle
    //         // RVCActuallyWriteText("idle popped", 11);
    //     }
    // }
    return retval;
}




//TODO: Call this frm rv/asm
//check mcause register == 5?
// if ^ then call hti -> schedule -> setup next interrupt -> jump into next thread -> back to beginning
// TCB* handle_timer_interrupt(){
//     SCHED_schedule();
// }

void setup_timer_interrupt(TTickRef time){
    
}

// catch interrupts function
// check mcause -> call function