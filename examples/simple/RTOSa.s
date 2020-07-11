;Jonathan Bayert
;
;Bayert RTOS system
;
;ECE 422 Project
;
;General Functions to run RTOS

	.cdecls C, list, "msp430.h"	; this allows us to use C headers

	.text						; locates code in 'text' section

	.global RTOSrun
	.global TimerLength
	.global taskSP
	.global currTask
	.global numTask
	.global RTOSrunReturn
	.global RTOSrestart
	.global mainSP
        .global currently_running

;this function is called to run the main code
RTOSrun:
	pushm.w #13, R15;save the current registers
	nop;
	mov.w SP,mainSP;save the main stack
	nop;continue on to the restart function
	mov.w #1d, currently_running; signal that the flag is currently running

RTOSrestart:
	;find the current stack
	mov.w #taskSP,R4
	add.w &currTask,R4
	add.w &currTask,R4


	nop;
	mov.w @R4, SP;move the stack pointer in
	nop;
	popm.w #13, R15;restore the current state
	nop;

	;start the timer
	mov.w &TimerLength, TA0CCR0;
	nop;
	eint;
	nop;

	reti;even though it is not in an interrupt
		;This still starts the first trial since the Stack pointer has changed

;exit the RTOSrun function
RTOSrunReturn:

	;return to the main stack
	nop
	mov.w &mainSP,SP;
	nop;

	popm.w #13, R15;restore the current state
	nop;

	;move the return value in
	mov.w #0d,R12;
	mov.w #0d,R13;
	nop;
	ret


;timer interrupt to switch the task
Timer0_ISR:

	pushm.w #13, R15;save the current state
	nop;

	;;find the current stack
	;;taskSP is the location where the SP are stored
	mov.w #taskSP,R4
	nop;
	add.w &currTask,R4
	add.w &currTask,R4

	mov.w SP, 0(R4);save the stack pointer

	;;should the taskSP be reset to task1
	cmp currTask, numTask
	jeq RESET

	;get the next value
	add.w #2d,R4;
	inc.w currTask;

CONTINUE:
	;update the pointers
	mov.w @R4, SP
	nop;
	popm.w #13, R15;restore the current state
	nop;

	reti

RESET:
	;reset value to task 1
	mov.w #0d, &currTask
	mov.w #taskSP,R4
	jmp CONTINUE




	nop
	.intvec   ".int44",  Timer0_ISR
	.end

