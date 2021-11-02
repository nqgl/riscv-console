#include "RVCOS.h"

// #include <stddef.h>
// #include <stdint.h>
// #include <stdlib.h>

volatile extern MemAddr threadPC;

typedef struct{
    uint32_t gp;
    uint32_t sp;
    // what else needs to be kept track of?
    //regs?
    uint32_t* children;
    TStatus status;
    MemAddr textcursor;
    int32_t cursor_position;
    TThreadID id;
} ThreadControlBlock, TCB, *ThreadControlBlockRef, *TCBref;


MemAddrRef sp; // set to mem locations of some
int tcbcount;
TCBref mainTCB;
TCB TCBs[256];
ThreadControlBlock myTCB;
ThreadControlBlockRef runningTCB = &myTCB;

#define TEXT_DATA_START ((MemAddr) 0x500FE800)
#define BACKGROUND_CONTROL_START ((MemAddr) 0x500FF100)
#define TEXT_LINE_WIDTH ((MemAddr) 0x500FE800)

TCBref handle_timer_interrupt(){

}

void runThread(TCBref thread){

}

void cursorCheck(){
    if (runningTCB->cursor_position >= 34*64){
        //push text up one level (w/ memcpy or w/e)
        runningTCB->cursor_position -= 64;
    }
    if (runningTCB->cursor_position < 0) {
        runningTCB->cursor_position = 0;

        //pr should I throw error? seems like a judgement call
    }    
}




TStatus take_char(TTextCharacter ch){
    if (ch == '\n'){
        runningTCB->cursor_position += 64;
        cursorCheck();
    }
    else if (ch == '\r'){
        unsigned int c = 11 % 3;
        runningTCB->cursor_position -= runningTCB->cursor_position % 64;
    }
    else if (ch == '\b'){
        runningTCB->cursor_position -= 1;
        cursorCheck();
    }
    else{
        volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xFE800);
        VIDEO_MEMORY[runningTCB->cursor_position] = ch;
        runningTCB->cursor_position += 1;
        cursorCheck();
    }
}
TTextCharacter eeee[200];


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

 
// "helper" function:  tohex
//  this was something I was working on in an effort to do some debugging
// but the function doesn't work if I recall correctly. 
// I don't know why i didn't just include something that has it
char* tohex(uint32_t uint){
    TTextCharacter* hexstr = eeee;
    hexstr[0] = '0';
    hexstr[1] = 'x';
    const TTextCharacter translate_str[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    for (int i = 0; i < 8; i++){
        hexstr[2+i] = translate_str[uint % 16];
        // uint /= 16;x
    }
    return hexstr;
}

TStatus RVCInitialize(uint32_t *gp){
    // RVCWriteText(tohex(*gp), 10);
    TStatus t = RVCOS_STATUS_SUCCESS;
    if (tcbcount != 0){
        return RVCOS_STATUS_ERROR_INVALID_STATE;
    } else {
        mainTCB = &TCBs[0];
        mainTCB->gp = *gp;
        return t;

    }


}



TStatus RVCTickMS(uint32_t *tickmsref){
    //todo
    return RVCOS_STATUS_ERROR_INSUFFICIENT_RESOURCES;
}
TStatus RVCTickCount(TTickRef tickref){
    //todo
    return RVCOS_STATUS_ERROR_INSUFFICIENT_RESOURCES;
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
TStatus RVCThreadCreate(TThreadEntry entry, void *param, TMemorySize memsize, TThreadPriority prio, TThreadIDRef tid){
    TCBref newThread = malloc(7);
    newThread->id = tcbcount;
    tcbcount++;
    return RVCOS_STATUS_ERROR_INSUFFICIENT_RESOURCES;
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
TStatus RVCThreadDelete(TThreadID thread){
    return RVCOS_STATUS_ERROR_INVALID_ID;
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
the newly created or dead sates, RVCOS_THREAD_STATE_CREATED or
RVCOS_THREAD_STATE_DEAD,
RVCOS_STATUS_ERROR_INVALID_STATE
is
returned.

*/
TStatus RVCThreadActivate(TThreadID thread){
    return RVCOS_STATUS_ERROR_INSUFFICIENT_RESOURCES;
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
TStatus RVCThreadTerminate(TThreadID thread, TThreadReturn returnval){
    return RVCOS_STATUS_ERROR_INSUFFICIENT_RESOURCES;
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
TStatus RVCThreadWait(TThreadID thread, TThreadReturnRef returnref){
    return RVCOS_STATUS_ERROR_INSUFFICIENT_RESOURCES;
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
    return 1;
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

TStatus RVCThreadState(TThreadID thread, TThreadStateRef state){
    //todo
    return RVCOS_STATUS_ERROR_INSUFFICIENT_RESOURCES;

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
#define RVCOS_TIMEOUT_INFINITE ((TTick)0)
#define RVCOS_TIMEOUT_IMMEDIATE ((TTick)-1)
TStatus RVCThreadSleep(TTick tick){
    if (tick == RVCOS_TIMEOUT_INFINITE){
        return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    }
    if (tick == RVCOS_TIMEOUT_INFINITE){
        //sit out this current quantum of time
    }

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
TStatus RVCWriteText(const TTextCharacter *buffer, TMemorySize writesize){
    runningTCB->cursor_position = 0;

    if (buffer == NULL) return RVCOS_STATUS_ERROR_INVALID_PARAMETER;
    for (int i = 0; i < writesize; i++){
        // TEXT IS BYTE-ADDRESSED //char==1byte
        take_char(buffer[i]);
    }
    return RVCOS_STATUS_SUCCESS;

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
TStatus RVCReadController(SControllerStatusRef statusref){
    //todo
    return RVCOS_STATUS_ERROR_INSUFFICIENT_RESOURCES;
}

