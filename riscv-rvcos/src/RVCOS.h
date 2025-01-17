#pragma once
#include <stdint.h>




#define RVCOS_STATUS_FAILURE                        ((TStatus)0x00)
#define RVCOS_STATUS_SUCCESS                        ((TStatus)0x01)
#define RVCOS_STATUS_ERROR_INVALID_PARAMETER        ((TStatus)0x02)
#define RVCOS_STATUS_ERROR_INVALID_ID               ((TStatus)0x03)
#define RVCOS_STATUS_ERROR_INVALID_STATE            ((TStatus)0x04)
#define RVCOS_STATUS_ERROR_INSUFFICIENT_RESOURCES   ((TStatus)0x05)
#define RVCOS_THREAD_STATE_CREATED                  ((TThreadState)0x01)
#define RVCOS_THREAD_STATE_DEAD                     ((TThreadState)0x02)
#define RVCOS_THREAD_STATE_RUNNING                  ((TThreadState)0x03)
#define RVCOS_THREAD_STATE_READY                    ((TThreadState)0x04)
#define RVCOS_THREAD_STATE_WAITING                  ((TThreadState)0x05)
#define RVCOS_THREAD_PRIORITY_IDLE                  ((TThreadPriority)0x00)
#define RVCOS_THREAD_PRIORITY_LOW                   ((TThreadPriority)0x01)
#define RVCOS_THREAD_PRIORITY_NORMAL                ((TThreadPriority)0x02)
#define RVCOS_THREAD_PRIORITY_HIGH                  ((TThreadPriority)0x03)

#define RVCOS_THREAD_ID_INVALID                     ((TThreadID)-1)
#define RVCOS_TIMEOUT_INFINITE                      ((TTick)0)
#define RVCOS_TIMEOUT_IMMEDIATE                     ((TTick)-1)
#define RVCOS_MEMORY_POOL_ID_SYSTEM                 ((TMemoryPoolID)0)
#define RVCOS_MEMORY_POOL_ID_INVALID                ((TMemoryPoolID)-1)
#define RVCOS_MUTEX_ID_INVALID                      ((TMutexID)-1)

typedef uint32_t TStatus, *TStatusRef;
typedef uint32_t TTick, *TTickRef;
typedef int32_t  TThreadReturn, *TThreadReturnRef;
typedef uint32_t TMemorySize, *TMemorySizeRef;
typedef uint32_t TThreadID, *TThreadIDRef;
typedef uint32_t TThreadPriority, *TThreadPriorityRef;
typedef uint32_t TThreadState, *TThreadStateRef;
typedef char     TTextCharacter, *TTextCharacterRef;
typedef uint32_t TMemoryPoolID, *TMemoryPoolIDRef;
typedef uint32_t TMutexID, *TMutexIDRef;
typedef TThreadReturn (*TThreadEntry)(void *);
typedef struct{
    uint32_t DLeft:1;
    uint32_t DUp:1;
    uint32_t DDown:1;
    uint32_t DRight:1;
    uint32_t DButton1:1;
    uint32_t DButton2:1;
    uint32_t DButton3:1;
    uint32_t DButton4:1;
    uint32_t DReserved:24;
} SControllerStatus, *SControllerStatusRef;
TStatus RVCInitialize(uint32_t *gp);
TStatus RVCTickMS(uint32_t *tickmsref);
TStatus RVCTickCount(TTickRef tickref);
TStatus RVCThreadCreate(TThreadEntry entry, void *param, TMemorySize memsize, 
TThreadPriority prio, TThreadIDRef tid);
TStatus RVCThreadDelete(TThreadID thread);
TStatus RVCThreadActivate(TThreadID thread);
TStatus RVCThreadTerminate(TThreadID thread, TThreadReturn returnval);
TStatus RVCThreadWait(TThreadID thread, TThreadReturnRef returnref, TTick timeout);
TStatus RVCThreadID(TThreadIDRef threadref);
TStatus RVCThreadState(TThreadID thread, TThreadStateRef stateref);
TStatus RVCThreadSleep(TTick tick);


TStatus RVCMutexCreate(TMutexIDRef mutexref);
TStatus RVCMutexDelete(TMutexID mutex);
TStatus RVCMutexQuery(TMutexID mutex, TThreadIDRef ownerref);
TStatus RVCMutexAcquire(TMutexID mutex, TTick timeout);
TStatus RVCMutexRelease(TMutexID mutex);
TStatus RVCWriteText(const TTextCharacter *buffer, TMemorySize writesize);
void RVCActuallyWriteText(const TTextCharacter *buffer, TMemorySize writesize);
TStatus RVCReadController(SControllerStatusRef statusref);



#define INTERRUPT_ENABLE_REG       (*((volatile uint32_t *) 0x40000000))
#define INTERRUPT_PENDING_REG       (*((volatile uint32_t *) 0x40000004))
#define MTIME_LOW       (*((volatile uint32_t *) 0x40000008)) //Ahh okay yeah these are pointers to memory locations
#define MTIME_HIGH      (*((volatile uint32_t *) 0x4000000C)) // right goddamn that makes sense okay
#define MTIMECMP_LOW    (*((volatile uint32_t *) 0x40000010)) //
#define MTIMECMP_HIGH   (*((volatile uint32_t *) 0x40000014))
#define CONTROLLER      (*((volatile uint32_t *) 0x40000018))

#define MBC_STATUS      (*((volatile uint32_t *) 0x40000018))  // Multi-Button Controller Status Register
#define CART_STATUS     (*((volatile uint32_t *) 0x4000001C))  // Cartridge Status Register
#define DMA_CH1_SRC     (*((volatile uint32_t *) 0x40000020))  // DMA Channel 1 Source Register
#define DMA_CH1_DEST    (*((volatile uint32_t *) 0x40000024))  // DMA Channel 1 Destination Register
#define DMA_CH1_CMD     (*((volatile uint32_t *) 0x40000028))  // DMA Channel 1 Command Register
#define DMA_CH1_STATUS  (*((volatile uint32_t *) 0x4000002C))  // DMA Channel 1 Status Register
#define DMA_CH2_SRC     (*((volatile uint32_t *) 0x40000030))  // DMA Channel 2 Source Register
#define DMA_CH2_DEST    (*((volatile uint32_t *) 0x40000034))  // DMA Channel 2 Destination Register
#define DMA_CH2_CMD     (*((volatile uint32_t *) 0x40000038))  // DMA Channel 2 Command Register
#define DMA_CH2_STATUS  (*((volatile uint32_t *) 0x4000003C))  // DMA Channel 2 Status Register
#define MCLK_PERIOD     (*((volatile uint32_t *) 0x40000040))  // Machine Clock Period Register
#define VCLK_PERIOD     (*((volatile uint32_t *) 0x40000044))  // Video Clock Period Register

