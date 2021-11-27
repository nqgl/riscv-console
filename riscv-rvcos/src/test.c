#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
volatile uint32_t controller_status = 0;
volatile uint32_t *saved_sp;
typedef void (*TFunctionPointer)(void);
void enter_cartridge(void);

int main(int argc, char const *argv[])
{
	PriorityQueue *q = PQ_create();
	char s10[] = "test10";
	char s20[] = "test20";
	char s30[] = "test30";
	char s11[] = "test11";
	char s21[] = "test21";
	char s31[] = "test31";
	char s12[] = "test12";
	char s22[] = "test22";
	char s32[] = "test32";
	PQ_put(q, s10, 0);
	PQ_put(q, s10, 2);
	PQ_put(q, s20, 0);
	PQ_put(q, s30, 3);
	PQ_put(q, s12, 0);
	PQ_put(q, s22, 1);
	PQ_put(q, s10, 0);
	PQ_put(q, s32, 0);

	while (q->size > 0){
		printf("q size:%i\n", q->size);
		printf("popped %s\n", ((char*)PQ_pop(q)));
	}

	return 0;
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
    TStatus status;
    if(code == 0x0){
        uint32_t* gp = p1;
        status = RVCInitialize( gp);
    }

    if(code == 0x1){
        status = RVCThreadCreate(((TThreadEntry) p1), p2, p3, p4, p5);
    }

    if(code == 0x2){
        status = RVCThreadDelete(((TThreadID) p1));
    }

    if(code == 0x3){
        status = RVCThreadActivate(((TThreadID) p1));
    }

    if(code == 0x4){
        status = RVCThreadTerminate(((TThreadID) p1), ((TThreadReturn) p2));
    }

    if(code == 0x5){
        status = RVCThreadWait(((TThreadID) p1), ((TThreadReturnRef) p2));
    }

    if(code == 0x6){
        status = RVCThreadID(((TThreadIDRef) p1));
    }

    if(code == 0x7){
        status = RVCThreadState(((TThreadID) p1), ((TThreadStateRef) p2));
    }

    if(code == 0x8){
        status = RVCThreadSleep(((TTick) p1));
    }

    if(code == 0x9){
        status = RVCTickMS(((uint32_t*) p1));
    }

    if(code == 0xA){
        status = RVCTickCount(((TTickRef) p1));
    }

    
    if(code == 0xB){
        const TTextCharacter* buffer = ((TTextCharacterRef) p1);
        status = RVCWriteText(buffer, ((TMemorySize) p2));
    }

    if(code == 0xC){
        status = RVCReadController(((SControllerStatusRef) p1));
    }


    return status; //I think this does correctly pass-by value it?
    return code + 1; //and I'm guessing that'll be what I actually should send rather than thiscode+1 return value
}


