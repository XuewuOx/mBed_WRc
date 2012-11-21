#ifndef MAIN_CPP
#define MAIN_CPP

#include "mbed.h"
#include "RTCfunc.h"
#include "COMfunc.h"
#include "stepmotor_ctr.h"
#include "SPIA2D.h"

#include "main.h"
#include "wtd.h"

 #include "MODSERIAL.h"

// Setup the watchdog timer
Watchdog wdt;

PwmOut led2(LED2);
float brightness = 0.0;

DigitalOut led1(LED1);
// DigitalOut led4debug(LED4);
DigitalOut led4(LED4);

Ticker flipper1;
// DigitalOut mheart(LED3); // LED3 has been used for heartbeat in RTCfunc.h/cpp


DigitalOut irGainCtr(p19);
DigitalOut uvGainCtr(p20);


extern int endofcmd; // -1 for normal char, 0 for 0x0D, 1 for 0x0A
                 // endofcmd =1 (TRUE) only when received 0D 0A ("\r\n")
extern MODSERIAL pc;

extern unsigned int Fs;


extern char ADCstatus; // 0 for idle,
                // 1 for coversion in progress,
                // 2 for maxSamples has been collected
                // 3  for continuous sampling
extern unsigned int a2dvalue[][2];


extern int nNow[NUMMOTOR];
extern float motorSpd[NUMMOTOR]; // steps per second
extern int statusLEDMotor; // 0 idle, 1 moving, 2 is at the dest, 3 is at the end

void dispCmdInfo();

int main()
{
	Fs=500;

    initCOMpc();
//    printf("INICOM...OK\n");
// On reset, indicate a watchdog reset or a pushbutton reset on LED 4 or 3
    if ((LPC_WDT->WDMOD >> 2) & 1)
        { led2 = 0.1; DEBUGF("\n\n!!--- WTD reset (LED2=0.1) ---!!\n");}
    else 
        { led1 = 1; DEBUGF("\n\n----- Power on reset (LED1=1) -----\n");}
    printf(" Compiled on %s at %s (Xuewu Daniel Dai)\n", __DATE__, __TIME__);
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
    
   dispCmdInfo();
    flipper1.attach(&heartbeat, 1.0); // the address of the function to be attached (flip) and the interval (2 seconds)
    wait(0.1);

      dispMotorStatus();
    wait(0.2);
    
    setMotor(MOTORIDLED, 0, 0, 100, FULLSTEP); // for LED
    setMotor(2, 0, 0, 1, FULLSTEP); // for LED
    moveMotor2Dest(MOTORIDLED, 20);
    
      // int account=0;
    while (1) {

        wdt.feed();
        if (endofcmd==1)
        {
         // pc.putc('A');
         cmdProcess();
         endofcmd=-1;
        }
        
        if (ADCstatus==2)
        { // required number of samples have been collected
           ADCstatus=0; // set 3 for continusou sampling,
                        // set 0 for one-shot sampling, see heartbeat()in RTCfunc.cpp
        }

        if (statusLEDMotor==2||statusLEDMotor==3)
        { // statusLEDMotor is set to 2 (arrives at the dest)
        	// and to 3 (arrives at the switch) in clkMotorLED()
            dispMotorStatus();
        	statusLEDMotor=0;
        	statusLEDMotor=0;

        }

        { // debug code for WTD
          //  account++;
          //  if (account>=1000)
          //  {while(1);
        }
        
       }// end of while(1)
} // end of main

void swingLED(int posA, int posB, int nSam)
{ 	int nSteps, dirMotor;
	int i,destTemp;
	float kk;
	float ms0; // motor speed temp

	printf("swing LED from A=%d to B=%d and collect %d UV/IR samples per step\n",posA, posB, nSam);
	if (nSam>MAXSAM)
		{ printf("Too many samples. a2dvalue will overflow. Reset nSam=%d",MAXSAM);
		nSam=MAXSAM;
		}
	// move motor one step before the starting position
	// Because, at each step, we will move motor one step first followed by collecting data.
	posA=posA-1;
    // move from nNow to posA at a fast speed
	ms0=motorSpd[MOTORIDLED];
    motorSpd[MOTORIDLED]=100; // 5 steps per second
    moveMotor2Dest(MOTORIDLED, posA);

    // set wait time for motor achieve posA
    nSteps=abs(posA-nNow[MOTORIDLED]);
    kk=(float) nSteps/motorSpd[MOTORIDLED]+2;  // steps per second
    if (kk>10.0)
    	wdt.kick(kk);
    // DEBUGF("wdt=%f s",kk);
    while(nNow[MOTORIDLED]!=posA)
    { //  printf(" %d ", nNow[MOTORIDLED]);
         wait(0.001);
    }
    // wait(1);
    // restor wait time and motor speed
    if (kk<10.0) wdt.kick(10.0);
    motorSpd[MOTORIDLED]=ms0; // restore the original value of motor speed
    // DEBUGF("\n, posA OK!\n");
    dispMotorStatus();

    dirMotor=posB-posA;
    nSteps=abs(dirMotor);
    for (i=0;i<MAXSAM;i++)
    	{a2dvalue[i][0]=4998;a2dvalue[i][1]=4999;}
    ADCstatus=0;
    // prefix sequence of data packet
    pc.printf("%%M posA=%d, nSteps=%d, nSam=%d, Fs=%d\r\n",posA, nSteps, nSam, Fs);
    /*
    // DEBUG codes for UART communicaiton with host PC/Beagle
    for (int i=0;i<=nSteps;i++)
    {
    	pc.printf("dir%03d=[",i);
    	for (int j=0;j<nSam;j++)
    		pc.printf(" %04d",1000*i+j);
    	pc.printf("]\r\n");

    	pc.printf("duv%03d=[",i);
    	for (int j=0;j<nSam;j++)
    	    		pc.printf(" %04d",1000*i+j);
    	pc.printf("]\r\n");
    	wait(0.1);
        wdt.feed();
    }

    return;
    // end of DEBUG codes for UART comm
*/
	startA2D(Fs,nSam);
	do{  wait(0.001);	} while(ADCstatus!=2);
    for (i=0; i<nSteps; i++)
    {
    	if (dirMotor==0)
    		{break;}
    	if (dirMotor>0)
    		{   moveMotornSteps(MOTORIDLED,1);
    			destTemp=nNow[MOTORIDLED]+1;
    		}
    	else
    		{   moveMotornSteps(MOTORIDLED,-1);
    			destTemp=nNow[MOTORIDLED]-1;
    		}
    	while(nNow[MOTORIDLED]!=destTemp)
    	{ //  printf(" %d ", nNow[MOTORIDLED]);
    		wait(0.001);
    	}
    	// wait(1);
    	// DEBUGF(" %d-th moveMotor2Dest, OK!\n", i+1);
    	startA2D(Fs,nSam);
    	do{  wait(0.001);	} while(ADCstatus!=2);
    	// send UV IR data up to host PC via USB-RS232
    	pc.printf("dir%03d=[",i);
    	for (int j=0;j<nSam;j++)
    		pc.printf(" %04d",a2dvalue[j][0]); // 0 for IR
    	pc.printf("]\r\n");

    	pc.printf("duv%03d=[",i);
    	for (int j=0;j<nSam;j++)
    	    		pc.printf(" %04d",a2dvalue[j][1]);// 1 for UV
    	pc.printf("]\r\n");
    	wait(0.1);

    	// DEBUGF("%d-th data collection, OK!\n", i+1);
    	wdt.feed();
    }
    // terminate sequence of data packet
    pc.printf("%% nROW=%d nCol=%d DATAIRUVEND\r\n",i,nSam);
    // pc.printf("true A2D values\r\n");
    dispMotorStatus();
    return;

}


void dispCmdInfo()
{
	 // pc.printf("Echoes back to the screen anything you type\r\n");
	    pc.printf("Command List\n");
	    pc.printf("  (0) resetmbed to reset the mBed by WTD\n");
	    pc.printf("  (1) 'u' to turn LED2 brightness up, 'd' to turn it down\r\n");

	    pc.printf("  (2) irs##<CR>  to start IR laser source\n");
	    pc.printf("      irt<CR>  to stop IR source and save its frq to EEROM\n");
	    pc.printf("      irf[+|-|#]###<CR>  to set frequency of IR LED current\n");
	    pc.printf("      irg###<CR>  to set IR's amplifier gain to ###\n");


	    pc.printf("  (3) uvs##<CR>  to start UV LED source\n");
	    pc.printf("      uvt<CR>  to stop UV source and save its frq to EEROM\n");
	    pc.printf("      uvf[+|-|#]###<CR>  to set frequency of UV LED current\n");
	    pc.printf("      uvg###<CR>  to set UV's amplifier gain to ###\n");

	    pc.printf("  (4) a2d type Fs nSamples<CR>\n");
	    pc.printf("          type = s  start a2d conversion and collect nSamples at Fs Hz\n");
	    pc.printf("          type = c  cancel ongoing a2d conversion\n");
	    pc.printf("          type = #  take one sample from #-th channel, here # is a number in [0,7]\n");

	    pc.printf("  (5)set and move motors\n");
	    dispMotorCmdHelp();


	    pc.printf("  (6) swn A B N<CR> swing source and collect UV/IR readings\n");
	    pc.printf("       move LED motor from A to B at preset motor parameters (see setm command)\n");
	    pc.printf("            and start collecting N UV/IR samples at each step (may not at the sampling rate Fs)\n");
	    pc.printf("            UV/IR data are sent back when motor arrives at position B\n");


	    pc.printf("\r\n");


}

#endif
