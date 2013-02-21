/*
 * main.h
 *
 *  Created on: 12 Oct 2012
 *      Author: Xuewu Daniel Dai 
 *       Email: xuewu.dai@eng.ox.ac.uk
 *         URL: http://users.ox.ac.uk/~engs1058/
 *
 */


#ifndef MAIN_H_
#define MAIN_H_

// #define DEBUG

#ifdef DEBUG
   #define DEBUGF printf
#else
    #define DEBUGF while(0)printf
#endif


/**    A structure for saving mBed's status into Flash memory
 *
 *       The IAP requires the number of bytes to be written should be 256 | 512 | 1024 | 4096,
 *       the size of the structure must be one of the above numbers.
 *       Table 578 "ISP Copy Command", Section 32.7.7 LPC1763_usermanual
 *
 *       For data types and size in mBed, http://mbed.org/handbook/C-Data-Types
 *       int 4 Bytes, unsigned int 4B,
 *       short 2B,  long 8B
 *       float 4B,  double 8B
 */

struct statusmbed{

   int nNow[3]; //nNow[1] for motorLED, nNow[2] for motorAPD, 12B
   unsigned int irm; // magnitude
   unsigned int irf; // frequency
   unsigned int irg; // gain

   unsigned int uvm;
   unsigned int uvf;
   unsigned int uvg;



   float APDbv; // bias voltage of APD (v), 4B
   unsigned int aomv; // analog output voltage (mv), 4B
unsigned int tempValue;

// size of data above: 48 Bytes = 11* sizeof(int) + 1 *sizeof(float)
//                              = 11*4 + 1*4
   char mempad[256-48];
   // int *p_statusLlEDMotor;
   // int *p_nNow;
   // float *p_motorSpd;

   // int *p_ADCstatus;
   // unsigned int *p_Fs;

};

void swingLED(int posA, int posB, int nSam);
void setAPDBiasVoltage(float bvAPD);
void setAnalogOut_mV(float ao_mv);
void dispmBedStatus();
void memdump( char *base, int n ) ;
int Initialize_main();
void testFlashMem();

#endif /* MAIN_H_ */
