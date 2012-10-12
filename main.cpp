#ifndef MAIN_CPP
#define MAIN_CPP

#include "mbed.h"
#include "RTCfunc.h"
#include "COMfunc.h"
#include "stepmotor_ctrl.h"
#include "SPIA2D.h"

#include "main.h"
#include "wtd.h"

 
// Setup the watchdog timer
Watchdog wdt;

PwmOut led2(LED2);
float brightness = 0.0;

DigitalOut led1(LED1);
// DigitalOut led4debug(LED4);
DigitalOut led4(LED4);

Ticker flipper1;
// DigitalOut mheart(LED3); // LED3 has been used for heartbeat in RTCfunc.h/cpp


extern int endofcmd; // -1 for normal char, 0 for 0x0D, 1 for 0x0A
                 // endofcmd =1 (TRUE) only when received 0D 0A ("\r\n")

                 
int main() {
	unsigned int Fs=500;

    initCOMpc();
//    printf("INICOM...OK\n");
// On reset, indicate a watchdog reset or a pushbutton reset on LED 4 or 3
    if ((LPC_WDT->WDMOD >> 2) & 1)
        { led2 = 0.1; DEBUGF("\n\n!!--- WTD reset (LED2=0.1) ---!!\n");}
    else 
        { led1 = 1; DEBUGF("\n\n----- Power on reset (LED1=1) -----\n");}
    printf(" Compiled on %s at %s (Xuewu Dai)\n", __DATE__, __TIME__);
    //    setTime(2012, 5, 6, 16, 11, 00);
    dispTime();
// setup a 10 second timeout on watchdog timer hardware
// needs to be longer than worst case main loop exection time
    wdt.kick(10.0);  
    printf("Start watch dog (feed interval < 10 s)... OK\n");
    
    Init_SPIMAX186();
    // wait(0.1);
    // DEBUGF("Hello world! by printf bug\r\n");
    // pc.printf("Hello world!\r\n\r\n");
    // mheart = 1;
    

    flipper1.attach(&heartbeat, 1.0); // the address of the function to be attached (flip) and the interval (2 seconds)
    wait(0.1);

    // pc.printf("Echoes back to the screen anything you type\r\n");
    pc.printf("Command List\n");
    pc.printf("  (0) resetmbed to reset the mBed by WTD\n");
    pc.printf("  (1) 'u' to turn LED2 brightness up, 'd' to turn it down\r\n");

    pc.printf("  (2) irs##<CR>  to start IR laser source\n");
    pc.printf("      irt<CR>  to stop IR source and save its frq to EEROM\n");
    pc.printf("      irf[+|-|#]###<CR>  to set frequency of IR LED current\n");

    pc.printf("  (3) uvs##<CR>  to start UV LED source\n");
    pc.printf("      uvt<CR>  to stop UV source and save its frq to EEROM\n");
    pc.printf("      uvf[+|-|#]###<CR>  to set frequency of UV LED current\n");

    pc.printf("  (4) set and move motors\n");
    dispMotorCmdHelp();
    pc.printf("\r\n");
    dispMotorStatus();
    wait(0.2);
    
    setMotor(MOTORIDLED, 0, 0, 1, FULLSTEP); // for LED
    setMotor(2, 0, 0, 1, FULLSTEP); // for LED
    moveMotor2Dest(MOTORIDLED, 5);
    
  
    // int account=0;
    while (1) {

        wdt.feed();
        if (endofcmd==1)
        {
         // pc.putc('A');
         cmdProcess();
         endofcmd=-1;
        }
        
        { // debug code for WTD
          //  account++;
          //  if (account>=1000)
          //  {while(1);
        }
        
       }// end of while(1)
    }

#endif
