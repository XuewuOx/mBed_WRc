/*
 * SPIA2D.cpp
 *
 *  Created on: 10 Oct 2012
 *      Author: dan
 */

#include "mbed.h"
#include "main.h"
#include "MODSERIAL.h"
#include "SPIA2D.h"

#define maxChnID 1
#define MAXSAM 2000


extern DigitalOut led1;
extern MODSERIAL pc;


SPI spimax186(p5, p6, p7); // mosi, miso, sclk
DigitalOut cs(p8);

// DigitalIn sstrb(p11);
InterruptIn ExIntr_sstrb(p11);

char adChn; // current A2D channel
unsigned int a2dvalue[MAXSAM][2]; //
unsigned int maxSamples; // total number of samples per channel to be collected
                         // 0 for continuous unlimited sampling
unsigned int nSamples; // how many samples have been collected

unsigned int Fs; // sampling rate

char ADCstatus; // 0 for idle,
                // 1 for coversion in progress,
                // 2 for maxSamples has been collected
                // 3  for continuous sampling

Ticker timerA2D;

void Intr_timerA2D(void);
void Intr_SSTRB(void);

void Init_SPIMAX186()
{
    // Setup the spi for 8 bit data, high steady state clock,
    // second edge capture, with a 1MHz clock rate
    spimax186.format(8,3);
    spimax186.frequency(1000000);
    // deSelect the device by seting chip select low
    cs = 1;
    ADCstatus=0;
    printf("Init SPI max186...OK3\n");
}

// Start A2D conversion, Fs sampling frequency
//               nSamples number of samples required
void startA2D(unsigned int xFs, unsigned int nSamplesRequired)
{
// DEBUGF("tartA2D... enter...");
if (ADCstatus==0||ADCstatus==2)
   {
   // DEBUGF("...ADCstatus==0");
     ExIntr_sstrb.rise(&Intr_SSTRB);
     adChn=0;
    nSamples=0;
    maxSamples=nSamplesRequired;
    Fs=xFs;
    ADCstatus=1; // dac conversion is in progress
    // timerA2D.attach(&Intr_timerA2D,0.002);
    timerA2D.attach(&Intr_timerA2D,(float)1/Fs);
    // DEBUGF("\n   timerA2D.attached ok, (ADCstatus=%d,%d samples at %d Hz (%fs))\n",ADCstatus, nSamplesRequired,Fs, (float) 1/Fs);
    }
else
    DEBUGF("Wrong ADCstatus=%d when invoking startA2D\n",ADCstatus);
}

void stopA2D()
{
   timerA2D.detach();   //    stopA2D();
   // ExIntr_sstrb.detach();
   ADCstatus=0; // set dac idle
}

// start A2D conversion,
// the number of A2D channels specified by maxChnID
// that is channel 0, 1,...maxChnID will be sampled
// the total number of samples per channel is specified by maxSamples
// stop timerA2D when the maxSamples is achieved
void Intr_timerA2D(void)
{
unsigned int i;
led1=!led1;
if (ADCstatus==1)
    if (nSamples>=maxSamples)
         {   // required number of data has been collected, send up to host and stop a2d
    	      timerA2D.detach();   //    stopA2D();
              ADCstatus=2; // maxSamples of samples have been collected
              // DEBUGF("%d samples done.\n[", nSamples);
              for (i=0;i<maxSamples;i++)
               {
                  printf("%04d ", a2dvalue[i][0]);
                    if (i%10==9)printf("\n");
               }
               // printf("]\n");
              // Now wait for main to process the ADCstatus
               // ADCstatus=0; // set 3 for continusou sampling,
                          // set 0 for one-shot sampling, see heartbeat()in RTCfunc.cpp
          }
    else
        {   // have not collected enough data, start next conversion
    	    cs=1;
            adChn=0;
            nSamples=nSamples+1;
            cs=0;
            spimax186.write(0x8E); // start CH0's conversion
            cs=1;
            // wait for sstrb interruption
            return;
        }
 else
 {
    DEBUGF("Wrong ADCstatus=%d when Intr_timerA2D interrupts\n",ADCstatus);
 }
}

// MAX186 completes data conversion for channel adChn
// if the conversion of last channel is done
// then stop conversion and wait for next timerA2D fires
void Intr_SSTRB(void)
{ char hi,lo;
  char ctrByte;
  unsigned int a2dtemp;

    if (ADCstatus!=1)
       return;
     cs=0;
     hi = spimax186.write(0x00);
     lo=spimax186.write(0x00);
     cs = 1;
     a2dtemp=hi<<8;
     a2dtemp=a2dtemp+lo;
     a2dtemp=a2dtemp>>3;
     a2dvalue[nSamples][(unsigned int)adChn]=a2dtemp;
     if (adChn>=maxChnID)
        {
          adChn=0;
          // ctrByte=0x8E; // select CH0
          // pc.printf("a2dvalue = [%dmv, %dmv (0x%X %X)]\n", a2dvalue[0],a2dvalue[1],hi,lo);
          return;
          }
     else
        { adChn=adChn+1;
          ctrByte=0xCE; // select CH1
          // start next AD conversion
          cs=0;
          spimax186.write(ctrByte);
          cs=1;
        }
}

void readA2D(char chn)
{
 char hi, lo;
 unsigned int a2dvalue;
  //   DEBUGF("star A2D conversion ch=%d...", chn);
 // select the max186
    cs=0;

// Write the control byte (1XXXXX11) to initiate a conversion
// and place the device into external clock mode.
// Control byte: START SEL2 SEL1 SEL0 UNI/BIP  SGL/DIF PD1 PD0
//                |-start AD conversion               |-Clk and powerdon mode
//                                                |- 1 Single ended, 0- diff
//                                       |- 1 unipolar  0 bipolar
//                      |-   |-    |- SEL2,1,0 select AD channel
// 0x8F: start CH0, unipolar, single ended, internal clock mode
    spimax186.write(0x8E);

 // The SSTRB(Serial Strobe Output) is monitored.

 // In internal clock mode, SSTRB
 // goes low  when the MAX186/MAX188 begin the A/D conversion and
 // goes high when the conversion is done.
 // In external clock mode, SSTRB pulses high for one clock period
 //  before the MSB decision.  A falling edge indicates that
 // the conversion is in progress and data is ready to be read
 //  High impedance when CS is high (external mode).
 cs=1; // deselect during conversation
// while(sstrb==0)
 while(ExIntr_sstrb==0)
 { }

    // Read in one data bit on each of the next 16 rising edges of SCLK.
    // These data bits represent the 12-bit conversion result followed
    // by four trailing bits, which should be ignored.
    cs=0;


     hi = spimax186.write(0x00);
     lo=spimax186.write(0x00);
     a2dvalue=hi<<8;
     a2dvalue=a2dvalue+lo;
     a2dvalue=a2dvalue>>3;
    pc.printf("a2dvalue[%d] = %dmv (0x%X %X) \n", chn,a2dvalue,hi,lo);
     // Deselect the device
    cs = 1;
 //   return a2dvalue;
 }


