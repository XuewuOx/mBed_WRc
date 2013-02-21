/*
 * SPITemp420.cpp
 *
 *  Created on: 21 Feb 2013
 *      Author: Xuewu Daniel Dai 
 *       Email: xuewu.dai@eng.ox.ac.uk
 *         URL: http://users.ox.ac.uk/~engs1058/
 * %              _
 * %  \/\ /\ /   /  * _  '
 * % _/\ \/\/ __/__.'(_|_|_
 */

#include "mbed.h"
#include "main.h"
#include "MODSERIAL.h"
#include "SPIA2D.h"
#include "SPITemp420.h"


SPI spitemp420(p5,p6,p7);
DigitalOut cstemp(p17);
DigitalOut cs420(p8);


Ticker timerRdTemp;

extern float tempDegC;

// extern MODSERIAL pc;

void Init_SPITemp420()
{
	  char cmdByte, cfgByte;

    // Setup the spi for 8 bit data, high steady state clock,
    // second edge capture, with a 1MHz clock rate
    spitemp420.format(8,3);
    spitemp420.frequency(10000);
    // deSelect the device by setting chip select high
    cs420 = 1;
    cstemp=1;

    // Initialize ADT7310
	cstemp=0;
    cmdByte=0x08; // to write cfg register
    cfgByte=0x40; //01000000, 1 SPS mode
    spitemp420.write(cmdByte);
    wait_us(10); // wait 10us for sending cmdByte
    spitemp420.write(cfgByte);
    wait_us(10); // wait 10us for sending cmdByte
   // cmdByte=0x54;// 01010100, continuous read
   // spitemp420.write(cmdByte);
    cstemp=1;


    // Initialize AD420



    printf("Init SPI temperature sensor & AD420...OK\n");
}

/** Read temperature once in single read mode
 *
 */
float readTemp()
{
	char cmdByte;
	char tempHi,tempLo;
	int tempReg16;
	double tempValue;

	cstemp=0;
	cmdByte=0x50;// 01010000
    spitemp420.write(cmdByte);
    tempHi=0x5A5A;
    tempLo=0xA5A5;
    tempHi=spitemp420.write(0x00);
    tempLo=spitemp420.write(0x00);
    cstemp = 1;
    tempReg16=tempHi<<8;
    tempReg16=tempReg16+tempLo;
    tempReg16=tempReg16&0xFFF8;
    tempValue=tempReg16;
    tempValue=tempValue/128.0;
    // DEBUGF("temperature=%8.4f, Hi=0x%02X Lo=0x%02X, Reg16=0x%04X\n", tempValue, tempHi,tempLo, tempReg16);
    return tempValue;
}


void Intr_timerTemp(void)
{
	tempDegC=readTemp();
}

/** Start continuous temperature reading at a fixed interval.
 *
 *  @param rdInterval_s the interval in second between two readings
 *
 */
void startReadingTemp(float rdInterval_s)
{
    timerRdTemp.attach(&Intr_timerTemp,rdInterval_s);
}


