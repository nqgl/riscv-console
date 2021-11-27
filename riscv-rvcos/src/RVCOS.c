#include "RVCOS.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// #include "memory_manager.h"
#include "double_queue.h"
#include "tcb.h"
#include "scheduler.h"
#include "debug.h"
#include "memory_manager.h"
#include "mutex.h"

volatile int cursor_position = 0;
volatile uint32_t global;
volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xFE800);

volatile extern uint32_t * threadPC;




extern TCB* mainTCB;
extern TCB* runningTCB;
extern TCB* idleTCB;

uint32_t _skeleton_function(void);
uint32_t ContextSwitch(volatile uint32_t** from, volatile uint32_t* to);


__attribute__((always_inline)) inline uint32_t csr_mstatus_read(void){
    uint32_t result;
    asm volatile ("csrr %0, mstatus" : "=r"(result));
    return result;
}

__attribute__((always_inline)) inline void csr_mstatus_write(uint32_t val){
    asm volatile ("csrw mstatus, %0" : : "r"(val));
}

__attribute__((always_inline)) inline void csr_write_mie(uint32_t val){
    asm volatile ("csrw mie, %0" : : "r"(val));
}

__attribute__((always_inline)) inline void csr_enable_interrupts(void){
    asm volatile ("csrsi mstatus, 0x8");
}

__attribute__((always_inline)) inline void csr_disable_interrupts(void){
    asm volatile ("csrci mstatus, 0x8");
}

void setNextTimeCmp(uint32_t interval);
typedef uint32_t (*TEntry)(uint32_t param);
uint32_t call_on_other_gp(void *param, TEntry entry, uint32_t *gp);

#define TEXT_DATA_START (*((volatile uint32_t *) 0x500FE800))
#define BACKGROUND_CONTROL_START (*((volatile uint32_t *) 0x500FF100))
#define TEXT_LINE_WIDTH (*((volatile uint32_t *) 0x500FE800))

#define VIDEO_MEMORY_BASE (((volatile char *) 0x50000000))
#define VIDEO_MEMORY_TEXT (((volatile char *) 0x50000000 + 0xFE800))
#define VIDEO_MEMORY_TEXT_END (((volatile char *) 0x50000000 + 0xFE800 + 0x8FF))




/*
RVCOS – Initializes cartridge main thread in RVCOS
-
RVCInitialize() initializes the cartridges main thread and sets the cartridges global pointer through
the gp variable. This system call be invoked as part of the cartridge startup code.
-
Upon successful initialization of the main thread, RVCInitialize() will return
RVCOS_STATUS_SUCCESS. If RVCInitialize() function is called after the initialization it will
return RVCOS_STATUS_ ERROR_INVALID_STATE.

*/

void idleFunc() {
    while(1) {
        continue;
    }
}

TThreadReturn idleEntry(void *a){
    idleFunc();
    return 0;
}


#define CONTROLLER_STATUS_REG   (*((volatile uint32_t *)0x40000018))

TStatus RVCInitialize(uint32_t *gp) {
    global = gp;
    SCHED_init();
    MUTEX_init();
    setNextTimeCmp(2);
    mainTCB->prio = RVCOS_THREAD_PRIORITY_NORMAL;
    mainTCB->state = RVCOS_THREAD_STATE_RUNNING;
    mainTCB = makeNewMainTCB(gp);
    TThreadIDRef tid;
    idleTCB = makeNewTCB(idleEntry, NULL, 1024, RVCOS_THREAD_PRIORITY_IDLE, tid);

    runningTCB = mainTCB;
    // RVCThreadActivate(0);
    //make sure main is fully initialized before calling activate
    RVCThreadActivate(1);

    // SCHED_add(idleTCB);
    return RVCOS_STATUS_SUCCESS;
}


TStatus RVCTickMS(uint32_t *tickmsref){
    //todo
    if (tickmsref==NULL){
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }
    *tickmsref = 2;
    return RVCOS_STATUS_SUCCESS;




}
extern volatile TTick tickTime;
TStatus RVCTickCount(TTickRef tickref){
    if (tickref==NULL){
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }
    *tickref = tickTime;
    return RVCOS_STATUS_SUCCESS;
}





/*
RVCThreadCreate – Creates a thread in the RVCOS.
-
RVCThreadCreate() creates a thread in the RVCOS. Once created the thread is in the created state
RVCOS_THREAD_STATE_CREATED. The entry parameter specifies the function of the
thread, and param specifies the parameter that is passed to the function. The size of the threads
stack is specified by memsize, and the priority is specified by prio. The thread identifier is put into
the location specified by the tid parameter.
-
Upon successful creation of the thread RVCThreadCreate() returns
RVCOS_STATUS_SUCCESS. If either entry or tid is NULL, RVCThreadCreate() returns
RVCOS_STATUS_ERROR_INVALID_PARAMETER.

*/
// typedef TThreadReturn (*TThreadEntry)(void *);

TStatus RVCThreadCreate(TThreadEntry entry, void *param, TMemorySize memsize, TThreadPriority prio, TThreadIDRef tid) {
    
    //todo I think we need to set up the sp and the stack space
    // in this function
    if (tid == NULL || entry == NULL) {
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    } else {
        // if (tcbcount >= 256){
        //     return RVCOS_STATUS_ERROR_INSUFFICIENT_RESOURCES;
        // }
        makeNewTCB(entry, param, memsize, prio, tid);
        return RVCOS_STATUS_SUCCESS;
        // // https://flaviocopes.com/c-array-length/
        // int arraySize = sizeof TCBArray / sizeof *TCBArray;
        // for(int i = 0; i<arraySize; i++) {
        //     if(TCBArray[i].tid == NULL) {
        //         TCBArray[i].entry = entry;
        //         TCBArray[i].param = param;
        //         TCBArray[i].memsize = memsize;
        //         TCBArray[i].prio = prio;
        //         TCBArray[i].tid = i;
        //         TCBArray[i].state = RVCOS_THREAD_STATE_CREATED;
        //         return RVCOS_STATUS_SUCCESS;
        //         break;
        //     }
        // }
    }
}


/*
RVCThreadDelete – Deletes a dead thread from the RVCOS.
-
RVCThreadDelete() deletes the dead thread specified by thread parameter from the RVCOS.
-
Upon successful deletion of the thread from the RVCOS, RVCThreadDelete() returns
RVCOS_STATUS_SUCCESS. If the thread specified by the thread identifier thread does not
exist, RVCOS_STATUS_ERROR_INVALID_ID is returned. If the thread does exist but is not in
the RVCOS_THREAD_STATE_DEAD state, RVCOS_STATUS_ERROR_INVALID_STATE
is returned.

*/



TStatus RVCThreadDelete(TThreadID thread) {

    TCB* threadTCB = getTCB(thread); 
    if (threadTCB == NULL){
        return RVCOS_STATUS_ERROR_INVALID_ID;
    } else if (threadTCB->state != RVCOS_THREAD_STATE_DEAD){
        return RVCOS_STATUS_ERROR_INVALID_STATE;
    } else {
        threadTCB->entry = NULL;
        threadTCB->param = NULL;
        // threadTCB->memsize = NULL;
        // threadTCB->prio = NULL;
        // threadTCB->tid = NULL;
        // threadTCB->state = NULL;
        return RVCOS_STATUS_SUCCESS;
    }
}




uint32_t *InitStack(uint32_t *sp, TThreadEntry skeleton_function, uint32_t param, TCB *tp, TThreadEntry entry_from_TCB){
        sp--;
        *sp = (uint32_t)skeleton_function; //sw ra,48(sp)
        sp--;
        *sp = ((uint32_t)tp);//sw tp,44(sp)
        sp--;
        *sp = 0;//sw t0,40(sp)
        sp--;
        *sp = 0;//sw t1,36(sp)
        sp--;
        *sp = 0;//sw t2,32(sp)
        sp--;
        *sp = 0;//sw s0,28(sp)
        sp--;
        *sp = 0;//sw s1,24(sp)
        sp--;
        *sp = param;//sw a0,20(sp)
        sp--;
        *sp = (uint32_t)entry_from_TCB;//sw a1,16(sp)
        sp--;
        *sp = tp->tid;//sw a2,12(sp)
        sp--;
        *sp = 0;//sw a3,8(sp)
        sp--;
        *sp = 0;//sw a4,4(sp)
        sp--;
        *sp = 0;//sw a5,0(sp)
    return sp;
}

//from piazza post https://piazza.com/class/ktlxmp9hob46o0?cid=640
void skeletonFunc(uint32_t param, TEntry entry, TThreadID tid){
    csr_enable_interrupts();
    TThreadReturn retval = call_on_other_gp(((void*)param), entry, global);
    csr_disable_interrupts();
    RVCThreadTerminate(tid, retval);
}



/*
RVCThreadActivate – Activates a newly created or dead thread in the RVCOS.
-
RVCThreadActivate() activates the newly created or dead thread specified by thread parameter in
the RVCOS. After activation the thread enters the RVCOS_THREAD_STATE_READY state,
and must begin at the entry function specified.
-
Upon successful activation of the thread in the RVCOS, RVCThreadActivate() returns
RVCOS_STATUS_SUCCESS. If the thread specified by the thread identifier thread does not
exist, RVCOS_STATUS_ERROR_INVALID_ID is returned. If the thread does exist but is not in
the newly created or dead states: (RVCOS_THREAD_STATE_CREATED or RVCOS_THREAD_STATE_DEAD,)
RVCOS_STATUS_ERROR_INVALID_STATE is returned.*/





TStatus RVCThreadActivate(TThreadID thread) {
    TCB* newThread = getTCB(thread); //misleading naming: the thread is new OR dead
    if (newThread == NULL){
        return RVCOS_STATUS_ERROR_INVALID_ID;
    }


    if (newThread->state == RVCOS_THREAD_STATE_CREATED){}
    else if (newThread->state == RVCOS_THREAD_STATE_DEAD){}
    else{
        return RVCOS_STATUS_ERROR_INVALID_STATE;
    }
    
    // uint8_t *ptr;
    // RVCMemoryAllocate(newThread->memsize, &ptr); //
    uint32_t *stack_top = (uint32_t *)(newThread->stack_top + newThread->memsize); // code from https://piazza.com/class/ktlxmp9hob46o0?cid=338
    newThread->saved_sp = InitStack(stack_top, skeletonFunc, ((uint32_t) newThread->param), newThread, newThread->entry); // took inspiration from https://piazza.com/class/ktlxmp9hob46o0?cid=338
    newThread->state = RVCOS_THREAD_STATE_READY;
    // runningTCB->state = RVCOS_THREAD_STATE_READY;
    // SCHED_add(runningTCB);//if not null ? maybe other conditions?
    SCHED_add(newThread);
    // SCHED_schedule();
    // maybe remove a copy of runningtcb?
    return RVCOS_STATUS_SUCCESS;
    // call scheduler? I don't think so,  I think scheduling happens elsewhere (part because this needs to return)
}


/*
RVCThreadTerminate – Terminates a thread in the RVCOS.
-
RVCThreadTerminate() terminates the thread specified by thread parameter in the RVCOS. After
termination the thread enters the state RVCOS_THREAD_STATE_DEAD, and the thread return
value returnval is stored for return values from RVCThreadWait(). The termination of a thread
can trigger another thread to be scheduled.
-
Upon successful termination of the thread in the RVCOS, RVCThreadTerminate() returns
RVCOS_STATUS_SUCCESS. If the thread specified by the thread identifier thread does not
exist, RVCOS_STATUS_ERROR_INVALID_ID is returned. If the thread does exist but is in the 
newly created or dead states, RVCOS_THREAD_STATE_CREATED or RVCOS_THREAD_STATE_DEAD, RVCOS_STATUS_ERROR_INVALID_STATE is returned.
*/


TStatus RVCThreadTerminate(TThreadID thread, TThreadReturn returnval) {
    TCB *terminateMe = getTCB(thread);
    if (terminateMe == NULL){
        return RVCOS_STATUS_ERROR_INVALID_ID;
    }
    if (terminateMe->state == RVCOS_THREAD_STATE_CREATED || terminateMe->state == RVCOS_THREAD_STATE_DEAD){
        return RVCOS_STATUS_ERROR_INVALID_STATE;
    }
    terminateMe->state = RVCOS_THREAD_STATE_DEAD;
    terminateMe->retval = returnval;
    // may 

    while(terminateMe->owned_mutexes->size > 0){
        Mutex *owned_mtx = dq_pop_back(terminateMe->owned_mutexes);
        MUTEX_release(owned_mtx);
    }

    TCB* waitingTCB = WAIT_pop_waiter(thread, WAIT_TYPE_THREAD);
    while(waitingTCB != NULL){
        *waitingTCB->return_value_storage_pointer = returnval;
        waitingTCB->waits_for = RVCOS_THREAD_ID_INVALID;
        waitingTCB->state = RVCOS_THREAD_STATE_READY;
        waitingTCB->waitsuccess = 1;
        SCHED_add(waitingTCB);
        // WAIT_wake_up(waitingTCB);
        waitingTCB = WAIT_pop_waiter(thread, WAIT_TYPE_THREAD);
    }
    SCHED_remove(terminateMe);


    return RVCOS_STATUS_SUCCESS;
}

/*
RVCThreadWait – Terminates a thread in the RVCOS.
-
RVCThreadWait() waits for the thread specified by thread parameter to terminate. The return
value passed with the associated RVCThreadTerminate() call will be placed in the location
specified by returnref. RVCThreadWait() can be called multiple times per thread.
-
Upon successful termination of the thread in the RVCOS, RVCThreadWait() returns
RVCOS_STATUS_SUCCESS. If the thread specified by the thread identifier thread does not
exist, RVCOS_STATUS_ERROR_INVALID_ID is returned. If the parameter returnref is NULL,
RVCOS_STATUS_ERROR_INVALID_PARAMETER is returned.

*/

    //regarding the following 3 functions
// TODO: Add state validity checking
// otherwise... anything else? probably activate will add thread to scheduler


//todo why does this need to be here
void randomfunc(){ //what the actual fuck how is this fixing anything at all what the fuck
    for (int i = 0; i < 4; i++){
        int j = 1;
    }
}

TStatus RVCThreadWait(TThreadID thread, TThreadReturnRef returnref, TTick timeout){
    TCB* waiteeTCB = getTCB(thread);
    if (waiteeTCB == NULL){
        return RVCOS_STATUS_ERROR_INVALID_ID;
    }
    if (timeout == RVCOS_TIMEOUT_INFINITE){
        runningTCB->waits_until = RVCOS_TIMEOUT_INFINITE;
    } else if (timeout == RVCOS_TIMEOUT_IMMEDIATE){
        if (waiteeTCB->state == RVCOS_THREAD_STATE_DEAD){
            *returnref = waiteeTCB->retval;    
            return RVCOS_STATUS_SUCCESS;
        }
        //todo why
        randomfunc(); //literal magic you just have to call this or else it doesnt work and it's impossible to know why that would be
        // RVCActuallyWriteText("state+wrong", 11);
        return RVCOS_STATUS_ERROR_INVALID_STATE; //probably this?
    } else{
        runningTCB->waits_until = tickTime + timeout;
    }

    if (returnref == NULL){
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }
    
    if (waiteeTCB->state != RVCOS_THREAD_STATE_DEAD) {
        runningTCB->state = RVCOS_THREAD_STATE_WAITING;
        runningTCB->return_value_storage_pointer = returnref;
        runningTCB->waits_for = thread;
        runningTCB->waitsuccess = 0;
        runningTCB->waittype = WAIT_TYPE_THREAD;
        WAIT_start_waiting();
        if (!runningTCB->waitsuccess){
            return RVCOS_STATUS_FAILURE;
        }
    }
    else{
        *returnref = waiteeTCB->retval;    
    }
        //put (waiting)id of this into some structure referencing (terminating) id
    // gets set in RVCTerminate
    
    return RVCOS_STATUS_SUCCESS;
}



/*
RVCThreadID – Retrieves thread identifier of the current operating thread.
-
RVCThreadID() puts the thread identifier of the currently running thread in the location specified
by threadref.
-
Upon successful retrieval of the thread identifier from the RVCOS, RVCThreadID() returns
RVCOS_STATUS_SUCCESS.
If
the
parameter
threadref
is
NULL,
RVCOS_STATUS_ERROR_INVALID_PARAMETER is returned.

*/
TStatus RVCThreadID(TThreadIDRef threadref){
    //todo
    if (threadref == NULL){
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }
    *threadref = runningTCB->tid;
    return RVCOS_STATUS_SUCCESS;
}




/*
RVCThreadState – Retrieves the state of a thread in the RVCOS.
-
RVCThreadState() retrieves the state of the thread specified by thread and places the state in the
location specified by state.
-
Upon successful retrieval of the thread state from the RVCOS, RVCThreadState() returns
RVCOS_STATUS_SUCCESS. If the thread specified by the thread identifier thread does not
exist, RVCOS_STATUS_ERROR_INVALID_ID is returned. If the parameter stateref is NULL,
RVCOS_STATUS_ERROR_INVALID_PARAMETER is returned.

*/


TStatus RVCThreadState(TThreadID thread, TThreadStateRef stateref) {
    TCB* thisTCB = getTCB(thread);
    if (thread == RVCOS_THREAD_ID_INVALID){
        return RVCOS_STATUS_ERROR_INVALID_ID;
    }
    if (thisTCB == NULL || thisTCB->tid == RVCOS_THREAD_ID_INVALID) {
        return RVCOS_STATUS_ERROR_INVALID_ID;
    }
    if (stateref == NULL) {
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }
    *stateref = thisTCB->state;
    return RVCOS_STATUS_SUCCESS;
}





/*
RVCThreadSleep – Puts the current thread in the RVCOS to sleep.
-
RVCThreadSleep() puts the currently running thread to sleep for tick ticks. If tick is specified as
RVCOS_TIMEOUT_IMMEDIATE the current process yields the remainder of its processing
quantum to the next ready process of equal priority.
-
Upon successful sleep of the currently running thread, RVCThreadSleep() returns
RVCOS_STATUS_SUCCESS.
If the sleep duration tick specified is RVCOS_TIMEOUT_INFINITE, 
RVCOS_STATUS_ERROR_INVALID_PARAMETER is
returned.

*/
extern volatile TTick tickTime;

TStatus RVCThreadSleep(TTick tick){
  
  
    if (tick == RVCOS_TIMEOUT_INFINITE){
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER; 
    }
    if (tick == RVCOS_TIMEOUT_IMMEDIATE){
        //sit out this current quantum of time
        //eg. schedule the next thread
        runningTCB->waits_until = tickTime;
    }
    else{
        runningTCB->waits_until = tick + tickTime;
    }
    runningTCB->state = RVCOS_THREAD_STATE_WAITING;
    SCHED_sleepRunning();


    return RVCOS_STATUS_SUCCESS;
}




/*
RVCWriteText – Writes text out to the RISC-V Console in text mode.
-
RVCWriteText() writes writesize characters starting at the location specified by buffer to the
RISC-V Console in text mode.
-
Upon successful writing of characters to the console RVCWriteText()
    returns RVCOS_STATUS_SUCCESS.
If the buffer parameter is NULL,
RVCOS_STATUS_ERROR_INVALID_PARAMETER is returned.
*/
int prev_was_nonspecial = 0;


void push_back_text(){
    volatile char * thisl = VIDEO_MEMORY_TEXT;
    while (((volatile char *) thisl + 64) <= VIDEO_MEMORY_TEXT_END){
        *thisl = *(thisl + 64);
        *(thisl + 64) = ' ';
        thisl++;
    }
}

void cursorCheck(){
    if (cursor_position >= 36*64){
        //push text up one level (w/ memcpy or w/e)
        push_back_text();
        cursor_position -= 64;
    }
    if (cursor_position < 0) {
        cursor_position = 0;

        //pr should I throw error? seems like a judgement call
    }    
}

// typedef int CSTATE;
// #define CURSOR_STATE_NORMAL 0
// #define CURSOR_STATE_ESC 1
// #define CURSOR_STATE_L_BRACKET 2
// #define 
// CSTATE cursor_state = 0;
// above is in case we need to make it do the state machine thing


TStatus take_char(TTextCharacter ch){
    cursorCheck();
    if (ch == '\n'){
        if (!(prev_was_nonspecial && cursor_position %64 == 64)){
            cursor_position += 64;  
            cursor_position -= cursor_position % 64; //this resets it to the beginning. Remove to reproduce typewriter mode.
            // cursor_position -= 1;
            prev_was_nonspecial = 0;
        }

    }
    else if (ch == '\r'){
        cursor_position -= cursor_position % 64;
        // cursorCheck();
        prev_was_nonspecial = 0;

    }
    else if (ch == '\b'){
        if (cursor_position % 64 != 0 || prev_was_nonspecial){ //don't backspace if at begiinning of a line
            cursor_position -= 1;
        }
        // cursorCheck();
        prev_was_nonspecial = 0;

    }
    else{
            // above replaced with #define++
        VIDEO_MEMORY_TEXT[cursor_position] = ch;
        cursor_position += 1;
        // cursorCheck();
        prev_was_nonspecial = 1; //
    }
}


#define KEYCODE_ESC ((char) 0x1b)
void move_cursor(line, col){
    cursor_position = 64*line + col;
}
void move_cursor_relative(line, col){
    int prev_line = cursor_position / 64;
    int prev_col = cursor_position % 64;
    move_cursor(line + prev_line, col + prev_col);
    // cursor_position += col;
    // cursor_position += line * 64;

}



void move_cursor_dumb(line, col){
    move_cursor(0,0);
    for (int i = 0; i < line; i++){
        move_cursor_relative(1, 0);
    }
    for (int i = 0; i < col; i++){
        move_cursor_relative(0, 1);
    }
}
void cursor_erase_screen_dont_move(){
    volatile char * thisl = VIDEO_MEMORY_TEXT;
    while (((volatile char *) thisl) <= VIDEO_MEMORY_TEXT_END){
        *(thisl) = ' ';
        thisl++;
    }

}

int char_to_number(char c){
    if (c == '0'){ return 0; }
    if (c == '1'){ return 1; }
    if (c == '2'){ return 2; }
    if (c == '3'){ return 3; }
    if (c == '4'){ return 4; }
    if (c == '5'){ return 5; }
    if (c == '6'){ return 6; }
    if (c == '7'){ return 7; }
    if (c == '8'){ return 8; }
    if (c == '9'){ return 9; }
    return -1;
}


TStatus RVCWriteText(const TTextCharacter *buffer, TMemorySize writesize){
    if (buffer == NULL) return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    VIDEO_wait_initiate(buffer, writesize);
    return RVCOS_STATUS_SUCCESS;
}

void RVCActuallyWriteText(const TTextCharacter *buffer, TMemorySize writesize){
    for (int i = 0; i < writesize; i++){
        // TEXT IS BYTE-ADDRESSED //char==1byte
        if (buffer[i] == KEYCODE_ESC){
            if (buffer[i+1] == '['){
                if (buffer[i+2] == 'A'){
                    move_cursor_relative(-1,0);
                    i += 3;
                    continue;
                }
                if (buffer[i+2] == 'B'){
                    move_cursor_relative(1,0);
                    i += 3;
                    continue;
                }
                if (buffer[i+2] == 'C'){
                    move_cursor_relative(0, 1);
                    i += 3;
                    continue;
                }
                if (buffer[i+2] == 'D'){
                    move_cursor_relative(0, -1);
                    i += 3;
                    continue;
                }
                if (buffer[i+2] == 'H'){
                    move_cursor(0,0);
                    i += 3;
                    continue;
                }
                if (buffer[i+3] == 'J' && buffer[i+2] == '2'){
                    cursor_erase_screen_dont_move();
                    i += 4;
                    continue;
                }
                i += 1;
                int line = 0;
                int next_num = 0;
                while (next_num != -1){
                    i += 1;
                    line = line * 10 + next_num;
                    next_num = char_to_number(buffer[i]);
                }
                if (buffer[i] == ';'){
                    int col = 0;
                    next_num = 0;
                    while (next_num != -1){
                        i += 1;
                        col = col * 10 + next_num;
                        next_num = char_to_number(buffer[i]);
                    }
                    if (buffer[i] == 'H'){
                        move_cursor(line, col);
                        i += 1;
                        continue;
                    }
                }
                else{
                    //malformed esc sequence
                }
            }
        }
        else{
        take_char(buffer[i]);
        }
    }

}




/*
RVCReadController – Reads the current status of the RISC-V Console controller.
-
RVCReadController() reads the current status of the RISC-V Console controller into the location
specified by statusref.
-
Upon successful reading the console controller status RVCReadController() returns
RVCOS_STATUS_SUCCESS.
If
the
statusref
parameter
is
NULL,
RVCOS_STATUS_ERROR_INVALID_PARAMETER is returned.

*/

TStatus RVCReadController(SControllerStatusRef statusref) {
    *(uint32_t *)statusref = CONTROLLER_STATUS_REG;

    if (statusref == NULL) {
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    } else {
        return RVCOS_STATUS_SUCCESS;
    }
}
/* RVCMemoryPoolCreate – Creates a memory pool from an array of memory.
Synopsys
#include "RVCOS.h"
TStatus RVCMemoryPoolCreate(void
memoryref);
*base,
TMemorySize
size,
TMemoryPoolIDRef
Description
RVCMemoryPoolCreate() creates a memory pool from an array of memory. The base and size of
the memory array are specified by base and size parameters respectively. The memory pool
identifier is put into the location specified by the memoryref parameter.
Return Value
Upon successful creation of the memory pool, RVCMemoryPoolCreate() will return
RVCOS_STATUS_SUCCESS. If the base or memoryref are NULL, or size is less than 128 bytes
(twice the minimum allocation) RVCOS_STATUS_ERROR_INVALID_PARAMETER is
returned.
This content is protected and may not be shared, uploaded, or distributed.
Project 3
15 of 30ECS150 FQ21
November 8, 2021
 */
 





/* RVCMemoryPoolDelete – Deletes a memory pool from the system.
Synopsys
#include "RVCOS.h"
TStatus RVCMemoryPoolDelete(TMemoryPoolID memory);
Description
RVCMemoryPoolDelete() deletes a memory pool that has no memory allocated from the pool.
The memory pool to delete is specified by the memory parameter.
Return Value
Upon successful deletion of the memory pool, RVCMemoryPoolDelete() will return
RVCOS_STATUS_SUCCESS. If the memory pool specified by memory is not a valid memory
pool, then RVCOS_STATUS_ERROR_INVALID_ID is returned. If memory is
RVCOS_MEMORY_POOL_ID_SYSTEM,
then
RVCOS_STATUS_ERROR_INVALID_PARAMETER is returned. If any memory has been
allocated
from
the
pool
and
not
deallocated,
then
RVCOS_STATUS_ERROR_INVALID_STATE is returned.
This content is protected and may not be shared, uploaded, or distributed.
Project 3
16 of 30ECS150 FQ21
November 8, 2021
 */
 



/* RVCMemoryPoolQuery – Queries the available space in the memory pool.
Synopsys
#include "RVCOS.h"
TStatus
RVCMemoryPoolQuery(TMemoryPoolID
TMemorySizeRef bytesleft);
memory,
Description
RVCMemoryPoolQuery() queries a memory pool for the available memory left in the pool. The
memory pool to query is specified by the memory parameter. The space left unallocated in the
memory pool is placed in the location specified by bytesleft.
Return Value
Upon successful querying of the memory pool, RVCMemoryPoolQuery() will return
RVCOS_STATUS_SUCCESS. If the memory pool specified by memory is not a valid memory
pool, then RVCOS_STATUS_ERROR_INVALID_ID is returned. If bytesleft is NULL,
RVCOS_STATUS_ERROR_INVALID_PARAMETER is returned. */






/* RVCMemoryPoolAllocate – Allocates memory from the memory pool.
Synopsys
#include "RVCOS.h"
#define
RVCMemoryAllocate(size,pointer)
RVCMemoryPoolAllocate(RVCOS_MEMORY_POOL_ID_SYSTEM, (size), (pointer))
TStatus
RVCMemoryPoolAllocate(TMemoryPoolID
TMemorySize size, void **pointer);
memory,
Description
RVCMemoryPoolAllocate() allocates memory from the memory pool. The memory pool to
allocate from is specified by the memory parameter. The size of the allocation is specified by size
and the base of the allocated array is put in the location specified by pointer. The allocated size
will be rounded to the next multiple of 64 bytes that is greater than or equal to the size parameter.
Return Value
Upon successful allocation from the memory pool, RVCMemoryPoolAllocate() will return
RVCOS_STATUS_SUCCESS. If the memory pool specified by memory is not a valid memory
pool, then RVCOS_STATUS_ERROR_INVALID_ID is returned. If size is zero or pointer is
NULL, RVCOS_STATUS_ERROR_INVALID_PARAMETER is returned. If the memory pool
does not have sufficient memory to allocate the array of size bytes,
RVCOS_STATUS_ERROR_INSUFFICIENT_RESOURCES is returned. */






/* RVCMemoryPoolDeallocate – Deallocates memory to the memory pool.
Synopsys
#include "RVCOS.h"
#define
RVCMemoryDeallocate(pointer)
RVCMemoryPoolDeallocate(RVCOS_MEMORY_POOL_ID_SYSTEM, (pointer))
TStatus
void *pointer);
RVCMemoryPoolDeallocate(TMemoryPoolID
memory,
Description
RVCMemoryPoolDeallocate() deallocates memory to the memory pool. The memory pool to
deallocate to is specified by the memory parameter. The base of the previously allocated array is
specified by pointer.
Return Value
Upon successful deallocation from the memory pool, RVCMemoryPoolDeallocate() will return
RVCOS_STATUS_SUCCESS. If the memory pool specified by memory is not a valid memory
pool, then RVCOS_STATUS_ERROR_INVALID_ID is returned. If pointer is NULL,
RVCOS_STATUS_ERROR_INVALID_PARAMETER is returned. If pointer does not specify a
memory location that was previously allocated from the memory pool,
RVCOS_STATUS_ERROR_INVALID_PARAMETER is returned. */








/* RVCMutexCreate– Creates a mutex in the system.
Synopsys
#include "RVCOS.h"
TStatus RVCMutexCreate(TMutexIDRef mutexref);
Description
RVCMutexCreate() creates a mutex in the system. Once created the mutex is in the unlocked state.
The mutex identifier is put into the location specified by the mutexref parameter.
Return Value
Upon
successful
creation
of
the
thread
RVCMutexCreate()
returns
RVCOS_STATUS_SUCCESS.
RVCMutexCreate()
returns
RVCOS_STATUS_ERROR_INVALID_PARAMETER if mutexref is NULL. If insufficient
resources
are
available
to
create
the
mutex
RVCOS_STATUS_ERROR_INSUFFICIENT_RESOURCES. */


TStatus RVCMutexCreate(TMutexIDRef mutexref){       
    if (mutexref == NULL){
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }
    //todo
    //if insufficient resources
        // return RVCOS_STATUS_ERROR_INSUFFICIENT_RESOURCES;

    Mutex* newtex = MUTEX_create();
    *mutexref = newtex->id;
    return RVCOS_STATUS_SUCCESS;
}


/* RVCMutexDelete – Deletes a mutex from the system.
Synopsys
#include "RVCOS.h"
TStatus RVCMutexDelete(TMutexID mutex);
Description
RVCMutexDelete() deletes the unlocked mutex specified by mutex parameter from the system.
Return Value
Upon successful deletion of the thread from the system, RVCMutexDelete() returns
RVCOS_STATUS_SUCCESS. If the mutex specified by the thread identifier mutex does not exist,
RVCOS_STATUS_ERROR_INVALID_ID is returned. If the mutex does exist, but is currently
held by a thread, RVCOS_STATUS_ERROR_INVALID_STATE is returned. */

TStatus RVCMutexDelete(TMutexID mutex){
    Mutex* mtx = getMutex(mutex);
    if (mtx == NULL){
        return RVCOS_STATUS_ERROR_INVALID_ID;
    }
    else if (mtx->owner != RVCOS_THREAD_ID_INVALID){
        return RVCOS_STATUS_ERROR_INVALID_STATE;
    }

    MUTEX_delete(mtx);
    return RVCOS_STATUS_SUCCESS;
}



/* RVCMutexQuery– Queries the owner of a mutex in the system.
Synopsys
#include "RVCOS.h"
TStatus RVCMutexQuery(TMutexID mutex, TThreadIDRef ownerref);
Description
RVCMutexQuery() retrieves the owner of the mutex specified by mutex and places the thread
identifier of owner in the location specified by ownerref. If the mutex is currently unlocked,
RVCOS_THREAD_ID_INVALID returned as the owner.
Return Value
Upon successful querying of the mutex owner from the system, RVCMutexQuery() returns
RVCOS_STATUS_SUCCESS. If the mutex specified by the mutex identifier mutex does not exist,
RVCOS_STATUS_ERROR_INVALID_ID is returned. If the parameter ownerref is NULL,
RVCOS_STATUS_ERROR_INVALID_PARAMETER is returned.
This content is protected and may not be shared, uploaded, or distributed. */

TStatus RVCMutexQuery(TMutexID mutex, TThreadIDRef ownerref){
    Mutex* mtx = getMutex(mutex);
    if (mtx == NULL){
        return RVCOS_STATUS_ERROR_INVALID_ID;
    }
    if (ownerref == NULL){
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }
    *ownerref = mtx->owner;
    return RVCOS_STATUS_SUCCESS;

}


/* RVCMutexAcquire – Locks the mutex.
Synopsys
#include "RVCOS.h"
#define RVCOS_TIMEOUT_INFINITE ((TTick)0)
#define RVCOS_TIMEOUT_IMMEDIATE ((TTick)-1)
TStatus RVCMutexAcquire(TMutexID mutex, TTick timeout);
Description
RVCMutexAcquire() attempts to lock the mutex specified by mutex waiting up to timeout ticks. If
timeout is specified as RVCOS_TIMEOUT_IMMEDIATE the current returns immediately if the
mutex is already locked. If timeout is specified as RVCOS_TIMEOUT_INFINITE the thread will
block until the mutex is acquired.
Return Value
Upon successful acquisition of the currently running thread, RVCMutexAcquire() returns
RVCOS_STATUS_SUCCESS. If the timeout expires prior to the acquisition of the mutex,
RVCOS_STATUS_FAILURE is returned. If the mutex specified by the mutex identifier mutex
does not exist, RVCOS_STATUS_ERROR_INVALID_ID is returned. */


TStatus RVCMutexAcquire(TMutexID mutex, TTick timeout){
    Mutex* mtx = getMutex(mutex);
    if (mtx == NULL){
        return RVCOS_STATUS_ERROR_INVALID_ID;
    }

    MUTEX_acquire(mtx, runningTCB, timeout);
    if (mtx->owner != runningTCB->tid){
        return RVCOS_STATUS_FAILURE;
    }
    return RVCOS_STATUS_SUCCESS;
}




/* RVCMutexRelease – Releases a mutex held by the currently running thread.
Synopsys
#include "RVCOS.h"
TStatus RVCMutexRelease(TMutexID mutex);
Description
RVCMutexRelease() releases the mutex specified by the mutex parameter that is currently held by
the running thread. Release of the mutex may cause another higher priority thread to be scheduled
if it acquires the newly released mutex.
Return Value
Upon
successful
release
of
the
mutex,
RVCMutexRelease()
returns
RVCOS_STATUS_SUCCESS. If the mutex specified by the mutex identifier mutex does not exist,
RVCOS_STATUS_ERROR_INVALID_ID is returned. If the mutex specified by the mutex
identifier mutex does exist, but is not currently held by the running thread,
RVCOS_STATUS_ERROR_INVALID_STATE is returned. */



TStatus RVCMutexRelease(TMutexID mutex){
    Mutex* mtx = getMutex(mutex);
    if (mtx == NULL){
        return RVCOS_STATUS_ERROR_INVALID_ID;
    }
    if (mtx->owner != runningTCB->tid){
        return RVCOS_STATUS_ERROR_INVALID_STATE;
    }
    MUTEX_release(mtx);
    return RVCOS_STATUS_SUCCESS;


}
