#include <stdint.h>
#include <sys/types.h>
#include "scheduler.h"

extern uint8_t _erodata[];
extern uint8_t _data[];
extern uint8_t _edata[];
extern uint8_t _sdata[];
extern uint8_t _esdata[];
extern uint8_t _bss[];
extern uint8_t _ebss[];


// extern uint8_t _heapbase[];
// extern uint8_t *_end;
// Adapted from https://stackoverflow.com/questions/58947716/how-to-interact-with-risc-v-csrs-by-using-gcc-c-code
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

uint32_t _sbrk(int incr) {
  extern char _heapbase;		/* Defined by the linker */
  static uint8_t *heap_end;
  char *prev_heap_end;
 
  if (heap_end == 0) {
    heap_end = &_heapbase;
  }
  prev_heap_end = heap_end;
  heap_end += incr;
  return ((uint32_t) prev_heap_end);
}

#define MTIME_LOW       (*((volatile uint32_t *) 0x40000008)) //Ahh okay yeah these are pointers to memory locations
#define MTIME_HIGH      (*((volatile uint32_t *) 0x4000000C)) // right goddamn that makes sense okay
#define MTIMECMP_LOW    (*((volatile uint32_t *) 0x40000010)) //
#define MTIMECMP_HIGH   (*((volatile uint32_t *) 0x40000014))
#define CONTROLLER      (*((volatile uint32_t *) 0x40000018))

void init(void){
    uint8_t *Source = _erodata;
    uint8_t *Base = _data < _sdata ? _data : _sdata;
    uint8_t *End = _edata > _esdata ? _edata : _esdata;

    while(Base < End){
        *Base++ = *Source++;
    }
    Base = _bss;
    End = _ebss;
    while(Base < End){
        *Base++ = 0;
    }   
    csr_write_mie(0x888);       // Enable all interrupt soruces
    csr_enable_interrupts();    // Global interrupt enable
    MTIMECMP_LOW = 1;
    MTIMECMP_HIGH = 0;
}

extern volatile int global;
extern volatile uint32_t controller_status;
volatile TTick tickTime = 0;
u_int32_t mcause_reg; //we need to set this later in the assembly
void setNextTimeCmp(uint32_t interval);


void c_interrupt_handler(void){
    int vipset = 0;
    if ((INTERRUPT_PENDING_REG & 0x2) > 0){
        vipset = 1;
    }


    if (mcause_reg == 0x80000007){
        uint64_t interval = 2;
        SCHED_schedule();
        uint64_t time = ((uint64_t) MTIME_HIGH) << 32;
        time += MTIME_LOW;
        time += interval;
        MTIMECMP_LOW = ((uint32_t) ((int32_t) -1));
        MTIMECMP_HIGH = (time >> 32);
        tickTime += 1;
        MTIMECMP_LOW = time;
        // global++;
        controller_status = CONTROLLER;
    } else if (mcause_reg == 0x8000000b && vipset){ //vip bit is video interrupt pending
        VIDEO_interrupt_write();
        INTERRUPT_PENDING_REG = INTERRUPT_PENDING_REG | 0x2;
    }

}

