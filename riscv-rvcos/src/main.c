// #include <stdint.h>
#include "RVCOS.h"
#include <stdint.h>

// #define MTIME_LOW       (*((volatile uint32_t *)0x40000008))
// #define MTIME_HIGH      (*((volatile uint32_t *)0x4000000C))
// #define MTIMECMP_LOW    (*((volatile uint32_t *)0x40000010))
// #define MTIMECMP_HIGH   (*((volatile uint32_t *)0x40000014))
// #define CONTROLLER      (*((volatile uint32_t *)0x40000018))



volatile int global = 42;
volatile uint32_t controller_status = 0;
volatile uint32_t *saved_sp;

typedef void (*TFunctionPointer)(void);
void enter_cartridge(void);
#define CART_STAT_REG (*(volatile uint32_t *)0x4000001C)
const TTextCharacter s[] = "stri\b\ng maybzz\rzz0000";
int main() {
    saved_sp = &controller_status;
    // RVCWriteText(s, ((TMemorySize)17));
    RVCInitialize(saved_sp);
    while(1){
        if(CART_STAT_REG & 0x1){
            enter_cartridge();
            break;
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
    if(code == 0){
        uint32_t* gp = p1;
        status = RVCInitialize(gp);
    }

    if(code == 1){
        status = RVCTickMS(((uint32_t*) p1));
    }

    if(code == 2){
        status = RVCTickCount(((TTickRef) p1));
    }

    if(code == 3){
        status = RVCThreadCreate(((TThreadEntry) p1), p2, p3, p4, p5);
    }

    if(code == 4){
        status = RVCThreadDelete(((TThreadID) p1));
    }

    if(code == 5){
        status = RVCThreadActivate(((TThreadID) p1));
    }

    if(code == 6){
        status = RVCThreadTerminate(((TThreadID) p1), ((TThreadReturn) p2));
    }

    if(code == 7){
        status = RVCThreadWait(((TThreadID) p1), ((TThreadReturnRef) p2));
    }

    if(code == 8){
        status = RVCThreadID(((TThreadIDRef) p1));
    }

    if(code == 9){
        status = RVCThreadState(((TThreadID) p1), ((TThreadStateRef) p2));
    }

    if(code == 10){
        status = RVCThreadSleep(((TTick) p1));
    }

    

    if(code == 11){
        const TTextCharacter* buffer = ((TTextCharacterRef) p1);
        status = RVCWriteText(buffer, ((TMemorySize) p2));
    }

    if(code == 12){
        status = RVCReadController(((SControllerStatusRef) p1));
    }


    csr_enable_interrupts();
    return status; //I think this does correctly pass-by value it?
    return code + 1; //and I'm guessing that'll be what I actually should send rather than thiscode+1 return value
}

