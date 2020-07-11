/*
 * Basic Demo of the RTOS feature
 *
 * Jon Bayert
 *
 *  April 13, 2020
 */

//include msp430 library
#include <msp430.h>

//import the RTOS library
#include "RTOS.h"

void task1();
void task2();
void task3();

//task one
//simple for loop
void task1(){
    int j;
    for(j = 0;j<600;j++){
        //insert code here
    }
}

//task two will call another task.
void task2(void)
{
    RTOSinitTask(task3);//call task 2

}

//task one
//another simple for loop
void task3(){
    int j;
    for(j = 0;j<400;j++){
        //insert code here
    }

}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer


    RTOSsetup();//set up the RTOS features
    int procid = RTOSinitTask(task1);//add task 1
    int procid2 = RTOSinitTask(task2);//add task 2
    int error = RTOSrun();//run the tasks
    //return here when all tasks finished
    //error is 0 when all task finished without errors.

    while(1);
    return 0;

}
