.syntax unified
.cpu cortex-m3
.thumb

.global g_pfnVectors
.global Reset_Handler
.global Default_Handler

.section .isr_vector, "a", %progbits
.type g_pfnVectors, %object
g_pfnVectors:
  .word _estack
  .word Reset_Handler
  .word Default_Handler
  .word Default_Handler
  .word Default_Handler
  .word Default_Handler
  .word Default_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word Default_Handler
  .word Default_Handler
  .word 0
  .word Default_Handler
  .word Default_Handler
.size g_pfnVectors, . - g_pfnVectors

.section .text.Reset_Handler, "ax", %progbits
.type Reset_Handler, %function
Reset_Handler:
  bl SystemInit
  bl main
1:
  b 1b
.size Reset_Handler, . - Reset_Handler

.section .text.Default_Handler, "ax", %progbits
.type Default_Handler, %function
Default_Handler:
2:
  b 2b
.size Default_Handler, . - Default_Handler

