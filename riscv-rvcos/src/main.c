// #include <stdint.h>
#include "RVCOS.h"
#include <stdint.h>
#include <stdio.h>
#include "debug.h"
#include "scheduler.h"
#include "memory_manager.h"
// #define MTIME_LOW       (*((volatile uint32_t *)0x40000008))
// #define MTIME_HIGH      (*((volatile uint32_t *)0x4000000C))
// #define MTIMECMP_LOW    (*((volatile uint32_t *)0x40000010))
// #define MTIMECMP_HIGH   (*((volatile uint32_t *)0x40000014))
// #define CONTROLLER      (*((volatile uint32_t *)0x40000018))



volatile uint32_t controller_status = 0;
volatile uint32_t *saved_sp;
volatile uint32_t *main_sp;
typedef void (*TFunctionPointer)(void);
void enter_cartridge(void);
#define CART_STAT_REG (*(volatile uint32_t *)0x4000001C)
int main() {
    saved_sp = &controller_status;
    MEMORY_init();
    while(1){
        if(CART_STAT_REG & 0x1){
            enter_cartridge();
            while(CART_STAT_REG & 0x1);
        }
    } 
    return 0;
}

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

void setNextTimeCmp(uint32_t interval){
    uint64_t time = ((uint64_t) MTIME_HIGH) << 32;
    time += MTIME_LOW;
    time += interval;
    MTIMECMP_LOW = ((uint32_t) ((int32_t) -1));
    MTIMECMP_HIGH = (time >> 32);
    MTIMECMP_LOW = time;
    
}


uint32_t c_syscall_handler(uint32_t p1,uint32_t p2,uint32_t p3,uint32_t p4,uint32_t p5,uint32_t code){
    csr_disable_interrupts();
    TStatus status;
    uint32_t* gp = p1;
    const TTextCharacter* buffer = ((TTextCharacterRef) p1);

    switch (code){
        case 0x0:
            status = RVCInitialize( gp);
            break;
        case 0x1:
            status = RVCThreadCreate(((TThreadEntry) p1), p2, p3, p4, p5);
            break;
        case 0x2:
            status = RVCThreadDelete(((TThreadID) p1));
            break;
        case 0x3:
            status = RVCThreadActivate(((TThreadID) p1));
            break;
        case 0x4:
            status = RVCThreadTerminate(((TThreadID) p1), ((TThreadReturn) p2));
            break;
        case 0x5:
            status = RVCThreadWait(((TThreadID) p1), ((TThreadReturnRef) p2), ((TTick) p3));
            break;
        case 0x6:
            status = RVCThreadID(((TThreadIDRef) p1));
            break;
        case 0x7:
            status = RVCThreadState(((TThreadID) p1), ((TThreadStateRef) p2));
            break;
        case 0x8:
            status = RVCThreadSleep(((TTick) p1));
            break;
        case 0x9:
            status = RVCTickMS(((uint32_t*) p1));
            break;
        case 0xA:
            status = RVCTickCount(((TTickRef) p1));
            break;
        case 0xB:
            status = RVCWriteText(buffer, ((TMemorySize) p2));
            break;
        case 0xC:
            status = RVCReadController(((SControllerStatusRef) p1));
            break;
        case 0xD:
            status = RVCMemoryPoolCreate((void*) p1, (TMemorySize) p2, (TMemoryPoolIDRef) p3);
            break;
        case 0xE:
            status = RVCMemoryPoolDelete((TMemoryPoolID) p1);
            break;
        case 0xF:
            status = RVCMemoryPoolQuery(p1, p2);
            break;
        case 0x10:
            status = RVCMemoryPoolAllocate(p1, p2, p3);
            break;
        case 0x11:
            status = RVCMemoryPoolDeallocate(p1, p2);
            break;
        case 0x12:
            status = RVCMutexCreate( ((uint32_t*)p1));
            break;
        case 0x13:
            status = RVCMutexDelete(p1);
            break;
        case 0x14:
            status = RVCMutexQuery(p1, ((uint32_t*)p2));
            break;
        case 0x15:
            status = RVCMutexAcquire(p1,p2);
            break;
        case 0x16:
            status = RVCMutexRelease(p1);
            break;
        
        }
    





    csr_enable_interrupts();    
    return status; //I think this does correctly pass-by value it?
    return code + 1; //and I'm guessing that'll be what I actually should send rather than thiscode+1 return value
}


