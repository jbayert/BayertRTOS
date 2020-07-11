/*
 * Jonathan Bayert
 *
 * ECE 422 Project 4
 *
 * Bayert RTOS system
 *
 * Import these functions into your project to use the RTOS system
 */


#ifndef RTOS_HEADER
#define RTOS_HEADER

//sets up the RTOS system
//call this before other commands
void RTOSsetup(void);

//This function adds a task to RTOS
//This can be done while the RTOS is running,
//so a task can create another task

//returns -1 is it could not add a task
//else returns a process id
int RTOSinitTask(void (*pFun)(void));

//runs all tasks.
//returns 0 when all tasks finished without errors.
int RTOSrun(void);


//the maximum number of tasks that will be called
#define MAX_TASKS 6

//the maximum size of the stack of each function
#define STACK_SIZE 40

//how often each task is called (in microseconds)
#define CYCLE_LENGTH 40000

//how many process ids might be created
//Even if a task has finished the process id will still
//be reserved for a bit so that the error code can be saved
#define MAX_PROCESSES 100

//The return statement for get proc_state
#define PROCESS_NOT_ALLOCATED 0//process not defined yet
#define PROCESS_NOT_STARTED 1//process has not started yet
#define PROCESS_RUNNING 2//the process is running
#define PROCESS_FINISHED_WITHOUT_ERRORS 3//process finished without any known errors

//returns whether a process is running
char get_proc_state(int process);

//stop a process from running with the process id
void killprocess(int proc_id);
#endif
