/*
 * Jonathan Bayert
 *
 * ECE 422 Project 4
 *
 * Bayert RTOS system
 *
 * General demo of the RTOS system.
 * This project demonstrates how to call functions from the main,
 * and also while the RTOS is running.
 * Also this shows how to kill a task
 */

#include <msp430.h>
#include "RTOS.h"
#include <driverlib.h> // Required for the LCD
#include "myGpio.h" // Required for the LCD
#include "myClocks.h" // Required for the LCD
#include "myLcd.h" // Required for the LCD
#include <stdlib.h>
#include <time.h>

#define SMCLK           0x0200          // Timer_A SMCLK source
#define RED_LED 0x0001 // P1.0 is the Red LED
#define STOP_WATCHDOG 0x5A80 // Stop the watchdog timer
#define ACLK 0x0100 // Timer ACLK source
#define UP 0x0010 // Timer Up mode
#define TACLR_ 0x0004 // Clear TAxR
#define ENABLE_PINS 0xFFFE // Required to use inputs and outputs

unsigned char HoldGreenLED =2;

void task1();
void task2();
void task3();
void task4();
void task5();


//This function displays a counter on the lcd screen
//When the button 1.1 is pressed the counter restarts
void task1(){

    PM5CTL0 = 0xFFFE; //activate I/O pins

    TA1CCR0 = 32768;     // setup TA1 timer to count for 1 second
    TA1CTL = 0x0114;     // start TA1 timer from zero in UP mode with ACLK

    initGPIO(); // Initializes Inputs and Outputs for LCD
    initClocks(); // Initialize clocks for LCD
    myLCD_init(); // Prepares LCD to receive commands

    P1DIR = P1DIR & ~0x0002; // P1.1 (Button S1) will be an input
    P1REN = P1REN | 0x0002; //enable pin 1.1
    P1OUT = P1OUT | 0x0002; //pin 1.1 is out
    int count = 0;
    while(1){
        if (TA1CTL & BIT0)     // check if timer finished counting
        {
            TA1CTL &= ~BIT0;     // reset timer flag
            count++;             // increment counter

            //display the current counter
            char num = count%10 +48;
            myLCD_showChar(num,6);
            num = (count/10)%10 +48;
            myLCD_showChar(num,5);

            num = (count/100)%10 +48;
            myLCD_showChar(num,4);


            //display the random number being used
            myLCD_showChar(' ',3);

            num = HoldGreenLED%10 +48;
            myLCD_showChar(num,2);

            num = (HoldGreenLED/10)%10 +48;
            myLCD_showChar(num,1);

        }//end if (TA2CTL & BIT0)
        if (!(0x0002 & P1IN))//isP1_1ButtonPressed
        {
            //display start
            myLCD_showChar(' ', 1);
            myLCD_showChar('S', 2);
            myLCD_showChar('T', 3);
            myLCD_showChar('A', 4);
            myLCD_showChar('R', 5);
            myLCD_showChar('T', 6);

            //delay
            int i,j;
            for(j=10;j>0;j--){
                for(i=20000;i>0;i--);
            }

            //display OVER
            myLCD_showChar(' ', 1);
            myLCD_showChar(' ', 2);
            myLCD_showChar('O', 3);
            myLCD_showChar('V', 4);
            myLCD_showChar('E', 5);
            myLCD_showChar('R', 6);

            //delay
            for(j=10;j>0;j--){
                for(i=20000;i>0;i--);
            }

            //start at 0
            myLCD_showChar(' ', 1);
            myLCD_showChar(' ', 2);
            myLCD_showChar(' ', 3);
            myLCD_showChar(' ', 4);
            myLCD_showChar(' ', 5);
            myLCD_showChar('0', 6);
            count = 0;
            TA1CTL &= ~BIT0;     // reset timer flag
        }
    }

}

//************************************************************************
// void Task2(void) Function
//
// Uses Timer TA2 to create a 1 HZ, 50% duty cycle
// signal on P9.7 (Green LED) and also every three
// seconds checks a global variable (HoldGreenLED)
// and will hold P9.7 high for the number of seconds
// corresponding to the interger contained in HoldGreenLED.
// Afterward, the 1 Hz 50% duty cycle signal resumes.
//************************************************************************
//
// Phil Walter
// 03/18/2020  v1
// for CE-422, Project 5
//************************************************************************

void task2(void)
{
// Setup - runs once
    unsigned char count = 0;   // local variable

    PM5CTL0 = 0xFFFE; //activate I/O pins
    P9DIR |= BIT7;       // make P9.7 (Green LED) an output
    P9OUT &= ~BIT7;      // Turn off P9.7 (Green LED)

    TA2CCR0 = 16384;     // setup TA2 timer to count for 0.5 second
    TA2CTL = 0x0114;     // start TA2 timer from zero in UP mode with ACLK

// loop that repeats forever - 1Hz 50% DC signl or on for HoldGreenLED seconds
    while(1)             // infinite loop
    {
// 1 Hz 50% DC signal
        if (TA2CTL & BIT0)     // check if timer finished counting
        {
            TA2CTL &= ~BIT0;     // reset timer flag
            P9OUT ^= BIT7;       // toggle P9.7 (Green LED)
            count++;             // increment counter
        }//end if (TA2CTL & BIT0)

// every three seconds get HoldGreenLED value and make P9.7 high
        if (count >= 6)        // 3 seconds elapsed?
        {
            TA2CCR0 = 32768;     // set timer to count for 1 second
            TA2CTL = 0x0114;     // start TA2 timer from zero in UP mode with ACLK
            P9OUT |= BIT7;       // Turn On P9.7 (Green LED)

            count = HoldGreenLED;     // set local variable equal to global variable
                                // how many seconds to keep P9.7 high
            while (count > 0)         // count down to zero
            {
                if (TA2CTL & BIT0)      // check if timer done counting to 1 second
                {
                    TA2CTL &= ~BIT0;      // reset timer flag
                    count--;              // decrement counter
                }
            }//end while(count > 0)

// Done with P9.7 high, go back to 1 Hz 50% DC signal
            TA2CCR0 = 16384;      // setup TA2 timer to count for 0.5 second
            TA2CTL = 0x0114;      // start TA2 timer from zero in UP mode with ACLK
            P9OUT ^= BIT7;        // start with P9.7 (Green LED) off

        }//end if (count >= 6)

    }//end while(1)

}//end Task2()


//Every 10 seconds a random value is put into the HoldGreenLED
//When button 1.1 task 4 is stated if it is not running
//When button 1.2 task 5 is killed and restated if it is not running
void task3(){
    PM5CTL0 = 0xFFFE; //activate I/O pins

    srand(time(0));//seed a need value
    int count = 0;

    TA3CCR0 = 32768;     // setup TA3 timer to count for 1 second
    TA3CTL = 0x0114;     // start TA3 timer from zero in UP mode with ACLK

    P1DIR = P1DIR & ~0x0004; // P1.2 (Button S2) will be an input
    P1REN = P1REN | 0x0004; //enable pin 1.2
    P1OUT = P1OUT | 0x0004; //pin 1.2 is out

    P1DIR = P1DIR & ~0x0002; // P1.1 (Button S1) will be an input
    P1REN = P1REN | 0x0002; //enable pin 1.1
    P1OUT = P1OUT | 0x0002; //pin 1.1 is out
    P1IN = 0x0;

    //the id of task created
    int task4_id = -1;
    int task5_id = -1;
    while(1){
        if (TA3CTL & BIT0)     // check if timer finished counting
        {
            TA3CTL &= ~BIT0;     // reset timer flag
            count++;             // increment counter
            if (count == 10){//ten seconds have passed
                HoldGreenLED = rand()%25+1;//update green led
                count = 0;
            }
        }

        if (!(0x0002 & P1IN))//isP1_1ButtonPressed
        {

            srand(time(0));//seed a need value

            if(task4_id == -1 ){//task4 has not been running
                task4_id= RTOSinitTask(task4);
            }else if (get_proc_state(task4_id)  == PROCESS_FINISHED_WITHOUT_ERRORS){//task4 is finished
                //start task 4
                task4_id= RTOSinitTask(task4);
            }
        }
        if (!(0x0004 & P1IN))//isP1_1ButtonPressed
        {

            srand(time(0));//seed a need value
            if(task5_id != -1){
                //the task has  been created
                killprocess(task5_id);//kill the task
                P1OUT &= ~BIT0;      // Turn off P9.7 (RED LED)
            }
            //delay
            int i,j;
            for(j=10;j>0;j--){
                for(i=20000;i>0;i--);
            }

            task5_id = RTOSinitTask(task5);//start task 5
            
        }
    }

}

//turns on the red LED for 1.5 seconds
void task4(){
    PM5CTL0 = 0xFFFE; //activate I/O pins
    P1DIR |= BIT0;       // make P9.7 (Red LED) an output
    P1OUT |= BIT0;      // Turn on P9.7 (RED LED)

    int i,j;
    for(j=30;j>0;j--){
        for(i=20000;i>0;i--);
    }

    P1OUT &= ~BIT0;      // Turn off P9.7 (RED LED) This should never happen

}

//turns on red LED and then enters low power mode
void task5(){
    PM5CTL0 = 0xFFFE; //activate I/O pins
    P1DIR |= BIT0;       // make P9.7 (Red LED) an output


    P1OUT |= BIT0;      // Turn on P9.7 (RED LED)
    _BIS_SR(LPM0_bits | GIE); // Enter Low Power Mode 0 and activate interrupts
    P1OUT &= ~BIT0;      // Turn off P9.7 (RED LED) This should never happen

}

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer


    //set up the RTOS
    RTOSsetup();

    //start up the tasks
    RTOSinitTask(task1);
    RTOSinitTask(task2);
    RTOSinitTask(task3);

    //run the system
    int error = RTOSrun();

    while(1);//loop here
    return 0;

}
