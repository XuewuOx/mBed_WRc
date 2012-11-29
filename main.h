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

struct statusmbed{
   unsigned int irm; // magnitude
   unsigned int irf; // frequency
   unsigned int irg; // gain

   unsigned int uvm;
   unsigned int uvf;
   unsigned int uvg;

   float APDbv; // bias voltage of APD (v)
   float aomv; // analog output voltage (mv)
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

#endif /* MAIN_H_ */
