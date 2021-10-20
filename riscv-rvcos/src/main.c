#include <stdint.h>
#include "RVCOS.h"
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
    RVCWriteText(s, ((TMemorySize)17));
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

    csr_enable_interrupts();
    return code + 1;
}



// #include <stdint.h>

// volatile int global = 42;
// volatile uint32_t controller_status = 0;

// volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xFE800);
// int main() {
//     int a = 4;
//     int b = 12;
//     int last_global = 42;
//     int x_pos = 12;

//     VIDEO_MEMORY[0] = 'H';
//     VIDEO_MEMORY[1] = 'e';
//     VIDEO_MEMORY[2] = 'l';
//     VIDEO_MEMORY[3] = 'l';
//     VIDEO_MEMORY[4] = 'o';
//     VIDEO_MEMORY[5] = ' ';
//     VIDEO_MEMORY[6] = 'W';
//     VIDEO_MEMORY[7] = 'o';
//     VIDEO_MEMORY[8] = 'r';
//     VIDEO_MEMORY[9] = 'l';
//     VIDEO_MEMORY[10] = 'd';
//     VIDEO_MEMORY[11] = '!';
//     VIDEO_MEMORY[12] = 'X';


//     while (1) {
//         int c = a + b + global; //this does...nothing?
        
//         if(global != last_global){
//             if(controller_status){
//                 VIDEO_MEMORY[x_pos] = ' ';
//                 if(controller_status & 0x1){
//                     if(x_pos & 0x3F){
//                         x_pos--;
//                     }
//                 }
//                 if(controller_status & 0x2){
//                     if(x_pos >= 0x40){
//                         x_pos -= 0x40;
//                     }
//                 }
//                 if(controller_status & 0x4){
//                     if(x_pos < 0x8C0){
//                         x_pos += 0x40;
//                     }
//                 }
//                 if(controller_status & 0x8){
//                     if((x_pos & 0x3F) != 0x3F){
//                         x_pos++;
//                     }
//                 }
//                 VIDEO_MEMORY[x_pos] = 'X';
//             }
//             last_global = global;
//         }
//     }
//     return 0;
// }
