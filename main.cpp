#ifndef MAIN_CPP
#define MAIN_CPP

#include "mbed.h"
#include "RTCfunc.h"
#include "COMfunc.h"
#include "stepmotor_ctr.h"
#include "SPIA2D.h"
#include "SPITemp420.h"
#include    "IAP.h"

#include "main.h"
#include "wtd.h"

 #include "MODSERIAL.h"


//----------------- iap flash memory access
#define     MEM_SIZE        256

#if defined(TARGET_LPC1768)
#define     TARGET_SECTOR    29     //  use sector 29 as target sector if it is on LPC1768
#elif defined(TARGET_LPC11U24)
#define     TARGET_SECTOR    7      //  use sector  7 as target sector if it is on LPC11U24
#define     TARGET_EEPROM_ADDRESS   64
#endif

void    memdump( char *p, int n );


IAP     iap;
//-------------------
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

AnalogOut APDBiasVoltage(p18);

extern int endofcmd; // -1 for normal char, 0 for 0x0D, 1 for 0x0A
                 // endofcmd =1 (TRUE) only when received 0D 0A ("\r\n")
extern MODSERIAL pc;

extern unsigned int Fs;


extern char ADCstatus; // 0 for idle,
                // 1 for coversion in progress,
                // 2 for maxSamples has been collected
                // 3  for continuous sampling

// extern unsigned int a2dvalue[][2]; // a2dvalue is discarded to remove limitation of maxum number of samples


extern int nNow[NUMMOTOR];
extern float motorSpd[NUMMOTOR]; // steps per second
extern int statusLEDMotor; // 0 idle, 1 moving, 2 is at the dest, 3 is at the end


statusmbed smbed;
float tempDegC; // temperature value (degree)


void dispCmdInfo();

int main()
{
    // setup a 10 second timeout on watchdog timer hardware
    // needs to be longer than worst case main loop exection time
    wdt.kick(10.0);
    printf("Start watch dog (feed interval < 10 s)... OK\n");

	Initialize_main();
wait(1);
	pc.printf("% main loop starts\r\n");
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

//============================================================
/** Initialize mBed
 *
 *  @param	  NONE
 *  @return     1   success
 *  		   -1	failure
 *  Remark:
 */
int Initialize_main()
{
    initCOMpc();
    // DEBUGF("INICOM...OK\n");
    // On reset, indicate a watchdog reset or a pushbutton reset on LED 4 or 3
    if ((LPC_WDT->WDMOD >> 2) & 1)
        { led2 = 0.1; DEBUGF("\n\n!!--- WTD reset (LED2=0.1) ---!!\n");}
    else 
        { led1 = 1; DEBUGF("\n\n----- Power on reset (LED1=1) -----\n");}
    printf(" Compiled on %s at %s (Xuewu Daniel Dai)\n", __DATE__, __TIME__);
    //setTime(2012, 5, 6, 16, 11, 00);
    dispTime();

    Init_SPIMAX186();
    Init_SPITemp420();
    // startReadingTemp(1.1);
    // printf("Start continuous temperature readings at 1.1 reading per second\n");
    testFlashMem();

    dispCmdInfo();
    flipper1.attach(&heartbeat, 1.0); // the address of the function to be attached (flip) and the interval (2 seconds)
    smbed.APDbv=140;
    pc.printf("%% set APD bias voltage to %3.2f v", smbed.APDbv);
    setAPDBiasVoltage(smbed.APDbv); // set APD bias voltage to 140 V
    wait(0.1);

    // dispMotorStatus();
    // wait(0.2);
    
    setMotor(MOTORIDLED, 0, 0, 200, FULLSTEP); // for  LED motor
    setMotor(2, 0, 0, 1, FULLSTEP); // for APD motor,
    // dispMotorStatus();
    wait(0.1);
    // LED motor power on test
    printf("%% LED motor power on test\r\n");
    moveMotor2Dest(MOTORIDLED, 100);
	wdt.feed();
    while(nNow[MOTORIDLED]!=100)
    {  wait(0.001);
 		if (statusLEDMotor==3) // stops when arrives at uSwitch, although not arrives at the dest
	   {printf("Motor stops when uSwitch is triggered.\r\n");
 		break;
	   }
    }
    wait(0.2);
    moveMotor2Dest(MOTORIDLED, 0);
    while(nNow[MOTORIDLED]!=0 )
    {  	wait(0.001);
     	if (statusLEDMotor==3) // stops when arrives at uSwitch, although not arrives at the dest
    	   {printf("Motor stops when uSwitch is triggered.\r\n");
     		break;
    	   }
    }
	wdt.feed();
	wait(0.2);

	dispMotorStatus();

	Fs=500;
	return EXIT_SUCCESS;

}




void testFlashMem()
{
    // char  mem[ MEM_SIZE ];    //  memory, it should be aligned to word boundary
    unsigned int     r;
    //--------------------------------
    printf( "Flash contents before write ...\r\n" );
    memdump( sector_start_adress[ TARGET_SECTOR ], MEM_SIZE);

    //  blank check: The mbed will erase all flash contents after downloading new executable

        r   = iap.blank_check( TARGET_SECTOR, TARGET_SECTOR );
        printf( "blank check result = 0x%08X, \" %s \" \r\n", r, r ? "FAILED" : "OK");
        printf( "-------------------------------------------------\r\n");
        //  erase sector, if required
/*
        if ( r == SECTOR_NOT_BLANK ) {
            iap.prepare( TARGET_SECTOR, TARGET_SECTOR );
            r   = iap.erase( TARGET_SECTOR, TARGET_SECTOR );
            printf( "erase result       = 0x%08X\r\n", r );
        }

        // copy RAM to Flash
*/
        smbed.irm=25; // magnitude
        smbed.irf=254; // frequency
        smbed.irg=1; // gain
        smbed.uvm=0;
        smbed.uvf=515;
        smbed.uvg=1;
        smbed.APDbv=141.0; // bias voltage of APD (v), 4B
        smbed.aomv=1515870810; // analog output voltage (mv), 4B, 5A5A5A5A
        smbed.nNow[0]=0x00000000; smbed.nNow[1]=0x00000001;smbed.nNow[2]=0x00000002;
        smbed.tempValue=0xA5A5A5A5;
        printf("aomv=0x%08X\r\n", smbed.aomv);
        for (r=0;r<sizeof(smbed.mempad);r++) smbed.mempad[r]=r;

            iap.prepare( TARGET_SECTOR, TARGET_SECTOR );
            // r   = iap.write( mem, sector_start_adress[ TARGET_SECTOR ], MEM_SIZE );
            // printf( "\r\n Copied: SRAM(0x%08X)->Flash(0x%08X) for %d bytes. (result=0x%08X)\r\n", mem, sector_start_adress[ TARGET_SECTOR ], MEM_SIZE, r );
            //            printf( "Flash contents after write mem[]=[0, 1, ...%d] \r\n", MEM_SIZE-1);
            //            memdump( sector_start_adress[ TARGET_SECTOR ], MEM_SIZE);
            r   = iap.write( (char*) &smbed, sector_start_adress[ TARGET_SECTOR ], sizeof(statusmbed) );

            printf( "\r\n Write smbed.nNow=[0x%08X, 0x%08X, 0x%08X] ->Flash for %d bytes. (result=0x%X), \"%s\" \r\n",
            		smbed.nNow[0],smbed.nNow[1],smbed.nNow[2], sizeof(statusmbed), r, r?"FAILED":"CMD_SUCCESS");

            printf( "Flash contents after write smbed \r\n");
            memdump( sector_start_adress[ TARGET_SECTOR ], sizeof(statusmbed));
            printf( "-------------------------------------------------\r\n");
wait(1);
     //       smbed.nNow[1]=7;
     //       for (r=0;r<sizeof(smbed.mempad);r++) smbed.mempad[r]=r+16;

            printf( "update smbed.nNow[1]=0x%08X, smbed.mempad[0]=0x%02X \r\n", smbed.nNow[1], smbed.mempad[0]);

            iap.prepare( TARGET_SECTOR, TARGET_SECTOR );
            r   = iap.write( (char*) &smbed, sector_start_adress[ TARGET_SECTOR ], sizeof(statusmbed) );
            printf( "Write new smbed to flash (result=0x%X), \"%s\" \r\n ", r, r?"FAILED":"CMD_SUCCESS" );
            printf( "Flash contents after write new smbed.nNow[1]");
            memdump( sector_start_adress[ TARGET_SECTOR ], sizeof(statusmbed));
            printf( "-------------------------------------------------\r\n");

            // compare
            r   = iap.compare( (char*) &smbed, sector_start_adress[ TARGET_SECTOR ], sizeof(statusmbed));
            printf( "compare result     = \"%s\"\r\n", r ? "FAILED" : "OK" );

    //--------------------------------
}

/** swing(scan) LED source to find IR/UV's peak position
 *
 *  @param    posA    Begin position for scanning.
 *  @param    posB    End position for scanning
 *  @param    nSam    Number of samples per step
 *  @return   NONE
 *  Remark:
 */
void swingLED(int posA, int posB, int nSam)
{ 	int nSteps, dirMotor;
	int i,destTemp;
	float kk;
	float ms0; // motor speed temp

	float delayAfterMovingMotor=0.2; // second
	printf("%% swing LED from A=%d to B=%d and collect %d UV/IR samples per step, %4f sec delay after moving motor \n",posA, posB, nSam, delayAfterMovingMotor);

	// a2dvalue is discarded to remove limitation of maximum number of samples
	// All A/D data is sent out via USB/RS232. Never store A/D data locally at mBed.
	// commented to avoid a2dvalue overflow
/*	 if (nSam>MAXSAM)
*		{ printf("%% Too many samples. a2dvalue will overflow. Reset nSam=%d",MAXSAM);
*		nSam=MAXSAM;
*		}
*/

	// move motor one step before the starting position
	// Because, at each step, we will move motor one step first followed by collecting data.
	posA=posA-1;
    // move from nNow to posA at a fast speed
	ms0=motorSpd[MOTORIDLED];
    motorSpd[MOTORIDLED]=200; // 5 steps per second
    moveMotor2Dest(MOTORIDLED, posA);

    // set wait time for motor achieve posA
    nSteps=abs(posA-nNow[MOTORIDLED]);
    kk=(float) nSteps/motorSpd[MOTORIDLED]+10;  // steps per second
    if (kk>10.0)
    	wdt.kick(kk);
    // DEBUGF("wdt=%f s",kk);
    while(nNow[MOTORIDLED]!=posA)
    { //  printf(" %d ", nNow[MOTORIDLED]);
         wait(0.001);
    }
    // wait(1);
    // restor wait time and motor speed
    wdt.kick(10.0);
    motorSpd[MOTORIDLED]=ms0; // restore the original value of motor speed
    // DEBUGF("\n, posA OK!\n");
    dispMotorStatus();

    dirMotor=posB-posA;
    nSteps=abs(dirMotor);

    ADCstatus=0;
    // prefix sequence of data packet
    pc.printf("%%SWN posA=%d, posB=%d, nSteps=%d, nSam=%d, Fs=%d\r\n",posA+1, posB, nSteps, nSam, Fs);
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
    // Start moving motor one step followed by a2d conversion
	// startA2D(Fs,nSam);
	// do{  wait(0.001);	} while(ADCstatus!=2);
    wait(0.01);
    pc.printf("%% header format \r\n%% motorStep IR1 UV1 IR2 UV2 IR3 UV3 ... \r\n");
    wait(0.01);
    pc.printf("%% DATAIRUVBEGIN nRow=%d nCol=%d", nSteps, nSam+1); // one more column for motor step
    wait(0.01);
    kk=(float) nSam/Fs+2;  // setup new dogfeeding interval
    if (kk>10.0) wdt.kick(kk);
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
    		wait(0.0005);
    	}
    	wait(delayAfterMovingMotor); // after moving the motor, wait for the signal being stable,
    	           // due to the transition response of electronic circuits
    	// DEBUGF(" %d-th moveMotor2Dest, OK!\n", i+1);
    	pc.printf("\r\n%04d ", destTemp);
    	startA2D(Fs,nSam);
    	do{  wait(0.0005);	} while(ADCstatus!=2);


    	// send UV IR data up to host PC via USB-RS232
    	/*
    	pc.printf("\ndir%04d=[",destTemp);
    	for (int j=0;j<nSam;j++)
    		pc.printf(" %04d",a2dvalue[j][0]); // 0 for IR
    	pc.printf("];\r\n");

    	pc.printf("duv%04d=[",destTemp);
    	for (int j=0;j<nSam;j++)
    	    		pc.printf(" %04d",a2dvalue[j][1]);// 1 for UV
    	pc.printf("];\r\n");
    	*/
    	wait(0.001);
  	// DEBUGF("%d-th data collection, OK!\n", i+1);
    	wdt.feed();
    }
	wdt.kick(10.0); // restore default value of dog feeding interval
    // terminate sequence of data packet
    pc.printf("\r\n%% nROW=%d nCol=%d DATAIRUVEND\r\n",i,nSam+1);
    // pc.printf("true A2D values\r\n");
    dispMotorStatus();
    return;

}


void setAPDBiasVoltage(float bvAPD)
{
	float k,b, mvAO;

	// k=21.0084; b=-12.2590;
	smbed.APDbv=bvAPD;
	k=17.0680;
	 b=517.3162;
	mvAO=bvAPD*k+b;
	setAnalogOut_mV(mvAO);
}

void setAnalogOut_mV(float ao_mv)
{
	 float per;

	 smbed.aomv=ao_mv;
	 per=ao_mv/3300.0; // 3.3v full range
	 APDBiasVoltage=per; // set analog output
	 pc.printf("\n%% OK analog output=%4.1fmv\n",ao_mv);
  /*  int i;
    while(1) {
    	 APDBiasVoltage = APDBiasVoltage + 0.01;
         wait_us(1);
         if(APDBiasVoltage == 1) {
        	 APDBiasVoltage = 0;
         }
     }
 */
}

void dispmBedStatus()
{
	printf("%% mBed status smbed: ");
	printf("irm=%d, irf=%d, irg=%d (p19)=%1d ",
			smbed.irm, smbed.irf, smbed.irg, irGainCtr.read());
	printf("uvm=%d, uvf=%d, uvg=%d (p20)=%1d APDbv=%4.2fv (p18=%dmv) ",
			smbed.uvm, smbed.uvf, smbed.uvg, uvGainCtr.read(), smbed.APDbv, smbed.aomv);
	dispMotorStatus();
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

	    pc.printf("  (7) apdbv #v<CR>  set the APD's bias voltage to # volts\n");
	    pc.printf("  (8) d2a #mv<CR>  set the mBed's analog output to # mini-volts\n");

	    pc.printf("\r\n");


}


void memdump( char *base, int n ) {
    unsigned char    *p;

    printf( "  memdump from 0x%08X for %d bytes", (unsigned int)base, n );

    p   = (unsigned char *)((unsigned int)base & ~(unsigned int)0x3);

    for ( int i = 0; i < n; i++, p++ ) {
        if ( !(i % 16) )
            printf( "\r\n  0x%08X :", (unsigned int)p );

        printf( " %02X", *p );
    }

    printf( "\r\n" );
}

#endif
