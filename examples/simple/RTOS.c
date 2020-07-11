/*
 * Jonathan Bayert
 *
 * ECE 422 Project 4
 *
 * Bayert RTOS system
 *
 * These functions in here are private and do not need to be altered for general use
 */

#include <msp430.h>
#include "RTOS.h"

#define SMCLK           0x0200          // Timer_A SMCLK source
#define RED_LED 0x0001 // P1.0 is the Red LED
#define STOP_WATCHDOG 0x5A80 // Stop the watchdog timer
#define ACLK 0x0100 // Timer ACLK source
#define UP 0x0010 // Timer Up mode
#define TACLR_ 0x0004 // Clear TAxR
#define ENABLE_PINS 0xFFFE // Required to use inputs and outputs

unsigned short TimerLength = CYCLE_LENGTH;//how long each task runs for

//Where all the stacks are stored
unsigned short taskStack[MAX_TASKS*STACK_SIZE];
//unsigned short StackOverFlowValue[MAX_TASKS];

unsigned short taskSP[MAX_TASKS];//the stack pointer of each stack
unsigned short taskProcID[MAX_TASKS];//the process of each task
unsigned short taskStackNumber[MAX_TASKS];//the stack of each task

unsigned short currTask;//the current task
unsigned short numTask;//the number of tasks. Note this is counts from 0. So there is actually 2 task when numTask = 1

//allocates a stack to be used by a task
//returns 0 with no task found
int alloc_stack(void);

//the location of the main stack, where to return after RTOSrun
unsigned short mainSP;

//0 is the not currently running
//1 is running
char currently_running;

char proc_array[MAX_PROCESSES];//the state of all ids



/*
 * private functions
 */
int alloc_proc(void);
void inline set_proc_state(int process, char state);
void inline set_proc_state_by_stack(char stack_num, char state);
char get_proc_stack(int process);
char get_proc_task_number(int process);
void taskFinished(void);

/*
 * private function in the RTOSa.s file
 */
void extern RTOSrunReturn(void);//when all tasks are finished this returns out of the RTOSrun function
void extern RTOSrestart(void);//restarts the RTOS when the timer has stopped (e.i a task has finished or a task was added)


//when tasks are finished they return here
//This removes the task and deallocates the stack
void killprocess(int proc_id){

    _BIC_SR(GIE); // Deactivate interrupts
    char proc_stack = get_proc_stack(proc_id);//get stack
    char task_num = get_proc_task_number(proc_id);//get task number

    if (task_num != 0xFF) {
        taskStack[((proc_stack+1)*STACK_SIZE) -1]=0;//make stack empty, so it can be reused

        set_proc_state_by_stack(task_num,PROCESS_FINISHED_WITHOUT_ERRORS);//set process state
        if(numTask == 0){
            if(currently_running) {
                RTOSrunReturn();//no stacks left
            }
            }
        }else{
            if(task_num == numTask){//this is the last need to update
                if(task_num==currTask) currTask--;
                numTask--;
            }else{
                //the task is in the middle
                int n;
                //need to shift the tasks over
                for(n=task_num;n<numTask;n++){
                    taskSP[n]=taskSP[n+1];
                    taskProcID[n]=taskProcID[n+1];
                    taskStackNumber[n]=taskStackNumber[n+1];
                }
                numTask--;
            }
        }
        TimerLength = CYCLE_LENGTH/(numTask+1);//update timer

        if(currently_running) {
            _BIS_SR(GIE);//enable interrupts again
        }

}



//returns -1 if it could not add a task
//returns a process id
int RTOSinitTask(void (*pFun)(void)){
    _BIC_SR(GIE); // Deactivate interrupts
    int add_Stask_location = alloc_stack();
    if(add_Stask_location == -1) return -1;

    int proc_id = alloc_proc();
    if(proc_id == -1) {
        return -1;
    }

    //unsigned short taskStart = (numTask+1)*STACK_SIZE
    unsigned short* taskStorage = &(taskStack[(add_Stask_location)*STACK_SIZE]);

    taskStorage[STACK_SIZE-3] = 0x09;//The SR
    taskStorage[STACK_SIZE-2] = (unsigned short) pFun;//The first PC
    taskStorage[STACK_SIZE-1] = (unsigned short) &taskFinished;//Return Value

    taskSP[(numTask+1)] =  (unsigned short) &(taskStorage[STACK_SIZE-16]);
    taskProcID[(numTask+1)] = proc_id;
    taskStackNumber[(numTask+1)]=add_Stask_location;

    numTask++;
    TimerLength = CYCLE_LENGTH/(numTask+1);
    
    if(currently_running){    
         TA0CCR0 = TimerLength; // Sets value of Timer_0

         _BIS_SR(GIE);

   }
   return proc_id;
}

//sets up the RTOS system
//call this before other commands
void RTOSsetup(void){
    mainSP = 0;
    TA0CCR0 = 0; // Sets value of Timer_0
    TA0CTL = SMCLK + UP + TACLR_; // Start TA0 from zero with SMCLK in UP MODE
    TA0CCTL0 = CCIE; // Enable interrupt for Timer_0

    numTask = -1;//there are 0 tasks
    currTask = 0;//start at the first task
    currently_running= 0;

    //clear the task stacks so they can be allocated
    int i;
    for(i= 0;i<MAX_TASKS;i++){
        taskStack[((i+1)*STACK_SIZE) -1]=0;
        taskProcID[i] = 0;
    }
    for(i=0;i<MAX_PROCESSES;i++){
        proc_array[i] =  PROCESS_NOT_ALLOCATED;
    }
}

/*
 * Private members follow here
 * Used caution when using or altering these.
 */


//when tasks are finished they return here
//This removes the task and deallocates the stack
void taskFinished(void){
    _BIC_SR(GIE); // Deactivate interrupts

    set_proc_state_by_stack(currTask,PROCESS_FINISHED_WITHOUT_ERRORS);
    asm(" mov.w #0d, 0(SP)");//make stack empty, so it can be reused
    asm(" mov.w #0d, R12");//make stack empty, so it can be reused
    asm(" push R12");//make stack empty, so it can be reused
    if(numTask == 0){
        RTOSrunReturn();
    }else{
        if(currTask == numTask){//this is the last need to update
            currTask--;
            numTask--;
        }else{
            //the task is in the middle
            int n;
            //need to shift the tasks over
            for(n=currTask;n<numTask;n++){
                taskSP[n]=taskSP[n+1];
                taskProcID[n]=taskProcID[n+1];
                taskStackNumber[n]=taskStackNumber[n+1];
            }
            numTask--;
        }
    }
    TimerLength = CYCLE_LENGTH/(numTask+1);//update timer

    RTOSrestart();//restart
}

//find and allocate a stack
//returns -1 if all the stacks are used
//else returns the stack number
int alloc_stack(void){
    int i;
    for(i= 0;i<MAX_TASKS;i++){
        if (taskStack[((i+1)*STACK_SIZE) -1]==0){//The stack is empty
            taskStack[((i+1)*STACK_SIZE) -1] = 1;//make sure another task doesn't go here
            return i;
        }

    }
    return -1;//no task is found
}

//find and allocate a stack
//returns -1 if all the processes are used
//else returns the stack number
int alloc_proc(void){
    int i;
    for(i=0;i<MAX_PROCESSES;i++){
        if (proc_array[i]==PROCESS_NOT_ALLOCATED){//The stack is empty
            proc_array[i]=PROCESS_RUNNING;
            return i;
        }
    }

    //if no empty processes clear old process
    for(i=0;i<MAX_PROCESSES;i++){
        if (proc_array[i]==PROCESS_FINISHED_WITHOUT_ERRORS){//The stack is empty
            proc_array[i]=PROCESS_RUNNING;
            return i;
        }
    }
    return -1;//no process is found
}

//sets the process state
void inline set_proc_state(int process, char state){
    proc_array[process] = state;
}

//get the task number based on a process id
//returns 0xFF if the process is not found running
char get_proc_task_number(int process){
    int i;
    for(i= 0;i<=numTask;i++){
        if(taskProcID[i] == process)  return i;
    }
    return 0xFF;
}

//get the stack number based on a process id
//returns 0xFF if the process is not found running
char get_proc_stack(int process){
    int i;
    for(i= 0;i<=numTask;i++){
        if(taskProcID[i] == process)  return taskStackNumber[i];
    }
    return 0xFF;
}

//get the process state
char get_proc_state(int process){
    return proc_array[process];
}

//set the state of a process. stack is the stack location of the process
//state is the new state
void inline set_proc_state_by_stack(char stack_num, char state){
    proc_array[taskProcID[stack_num]] = state;
}
