; armfly-v7 / STM32H743XIH6
; Minimal startup scaffold for STM32H743XIH6. Replace with the vendor startup file before
; building a production image if your project needs the full IRQ vector table.

Stack_Size      EQU     0x00001000
Heap_Size       EQU     0x00000000

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit

                PRESERVE8
                THUMB

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

                IMPORT  NMI_Handler
                IMPORT  HardFault_Handler
                IMPORT  MemManage_Handler
                IMPORT  BusFault_Handler
                IMPORT  UsageFault_Handler
                IMPORT  SVC_Handler
                IMPORT  DebugMon_Handler
                IMPORT  PendSV_Handler
                IMPORT  SysTick_Handler
                IMPORT  SystemInit
                IMPORT  __main

__Vectors
                DCD     __initial_sp
                DCD     Reset_Handler
                DCD     NMI_Handler
                DCD     HardFault_Handler
                DCD     MemManage_Handler
                DCD     BusFault_Handler
                DCD     UsageFault_Handler
                DCD     0
                DCD     0
                DCD     0
                DCD     0
                DCD     SVC_Handler
                DCD     DebugMon_Handler
                DCD     0
                DCD     PendSV_Handler
                DCD     SysTick_Handler
__Vectors_End
__Vectors_Size  EQU     __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY

Reset_Handler   PROC
                EXPORT  Reset_Handler [WEAK]
                BL      SystemInit
                BL      __main
                ENDP

                ALIGN
                END
