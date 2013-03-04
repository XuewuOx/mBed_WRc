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
DigitalOut cs420A(p8); // CS for 1st 4-20mA output
DigitalOut cs420B(p16); // CS for 2nd 4-20mA output

// DigitalOut clck(p7);

Ticker timerRdTemp;

extern float tempDegC;

// extern MODSERIAL pc;

void Init_SPITemp420()
{
	  char cmdByte, cfgByte;

    // Setup the spi for 8 bit data, high steady state clock,
    // second edge capture, with a 1MHz clock rate
    spitemp420.format(8,0);
    spitemp420.frequency(5000);
    // deSelect the device by setting chip select high
    cs420A=1;
    cs420B=1;
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
/*  4-20mA test codes
*/

/*
 while(1)
{
//	readTemp();
//	wait_ms(10);
	// set420mAOutput(4.000488, 1);
	set420mAOutput(4.000488, 1);
	// set420mAOutput(8, 2);
	wait_ms(25);
	set420mAOutput(8.000488, 1);
	wait_ms(25);
	set420mAOutput(10.000, 1);
	wait_ms(25);
	set420mAOutput(14.000, 1);
	wait_ms(25);
	set420mAOutput(18.000, 1);
	wait_ms(25);
	set420mAOutput(19.000, 1);
	wait_ms(75);
}
*/
    printf("Init SPI temperature sensor & AD420...OK\n");
}


/** Set 4-20mA output
 *
 *  @param curr		current to be set in mA
 *  @param chID		1 for first 4-20mA output channel, 2 for 2nd channel
 *
 */
int set420mAOutput(float curr, int chID)
{
	unsigned currCode;
	char currHi, currLo;

	if (curr<=4)
		currCode=0;
	else if (curr>=20)
		currCode=0xFFFF;
	else
		currCode=round((curr-4)/0.000244140625);
	currLo=currCode&0x00FF;
	currHi=currCode>>8;

	// currLo=currLo^0xFF;
	// currHi=currHi^0xFF;
	printf("Set %9.6f mA to %d-th channel, Hi=0x%02X, Lo=0x%02X\r\n", curr, chID, currHi, currLo);
	if (chID==1)
	{
		cs420A=0;
		spitemp420.write(currHi);
		spitemp420.write(currLo);
		wait_us(200);
		cs420A=1;
		return 1;
	}
	else if (chID==2)
	{
		cs420B=0;
		spitemp420.write(currHi);
		spitemp420.write(currLo);
		wait_us(200);
		cs420B=1;
		return 2;
	}
	else
	{   printf("Wrong 4-20mA channel ID that should be in {1, 2}\r\n");
		return -1;
	}
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
    printf("temperature=%8.4f, Hi=0x%02X Lo=0x%02X, Reg16=0x%04X\n", tempValue, tempHi,tempLo, tempReg16);
    return tempValue;
}


void Intr_timerTemp(void)
{
	tempDegC=readTemp();
}

/** Start continuous temperature reading at a fixed interval.
 *
 *  @param rdInterval-_s the interval in second between two readings
 *
 */
void startReadingTemp(float rdInterval_s)
{
    timerRdTemp.attach(&Intr_timerTemp,rdInterval_s);
}



