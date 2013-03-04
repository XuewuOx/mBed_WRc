/*
 * stepmotor_ctr.cpp
 *
 *  Created on: 12 Oct 2012
 *      Author: Xuewu Daniel Dai 
 *       Email: xuewu.dai@eng.ox.ac.uk
 *         URL: http://users.ox.ac.uk/~engs1058/
 *
 */

#include "stepmotor_ctr.h"

#include "MODSERIAL.h"
#include "main.h"

#define uSW_OpticRef 1
#define uSW_WaterMes 0

#define uSW_POSITION  0
// uSWU_POSITION   1 when the uSW is at the optic reference end
//                 0 when the uSW at the water measurement end

/* -------------
* external variable declaration
*/
extern MODSERIAL pc;
extern DigitalOut led4;
extern DigitalOut led1;

extern statusmbed smbed;

/* -------------
* variable definition
*/
DigitalOut clkLED(p22);
DigitalOut enbLED(p24);
DigitalOut dirLED(p23); // 0 clockwise, 1 anti-clockwise
// different from Matlab, due to xor in matlab
DigitalOut fulLED(p21); // used to be 27, changed for serial comm

DigitalOut clkAPD(p25);
DigitalOut enbAPD(p26);
DigitalOut dirAPD(p30); // used to be 29, p29 now for microSWitch
DigitalOut fulAPD(p30); // used to be 28, changed to 30 for serial comm

DigitalOut *pCLK[2];
DigitalOut *pENB[2];
DigitalOut *pDIR[2];
DigitalOut *pSTP[2];

DigitalIn uSW(p29); // =1 when the LED motor is at the end of the rail
                    // =0 otherwise
int statusLEDMotor; // 0 idle, 1 moving, 2 is at the dest, 3 is at the end


float motorSpd[NUMMOTOR]; // steps per second
bool fullStep[NUMMOTOR]; // 1 for full step, 0 for half step
int nDest[NUMMOTOR];
int nDest2_LED;

int nOrigin[NUMMOTOR];
int nNow[NUMMOTOR];

Ticker tickerLED; // tickerLED.timeout= 0.5 *(1/speedLED)
Ticker tickerMotor[2];



/* -------------
* Function definition
*/

void dispMotorCmdHelp() {
    pc.printf("      setm motorID nOrigin nNow speed fullStep   :set motor's parameters, motorID=1 for LEDmotor, =2 for APDmotor\n");
    pc.printf("          setm without parameters to display motor status\r\n");
    pc.printf("      move -d|s motorID nStep        : move the motor with motorID \r\n");
    pc.printf("        -d move motor[motorID] to position of nStep \n");
    pc.printf("        -s move motor[motorID] nStep steps. \n");
    pc.printf("          If nStep>0, move forward; if nStep<-0, move backward\r\n");
}

void dispMotorStatus() {
    pc.printf("%% motor[1] is motorLED: nOrigin=%d, nNow=%d, motorSpd=%3.2f steps/s, fullStep=%d, ",nOrigin[1], nNow[1],motorSpd[1],fullStep[1]);
    pc.printf("statusLEDMotor=%d, uSW(p29)=%d\n", statusLEDMotor, (int) uSW);
   // pc.printf("motor[2] is motorAPD: nOrigin=%d, nNow=%d, motorSpd=%3.2f steps/s, fullStep=%d\r\n",nOrigin[2], nNow[2],motorSpd[2],fullStep[2]);

}

void setMotor(int motorID, int nO, int nN, float spd, int fullstep) {
    int k;

    statusLEDMotor=0;

    pCLK[0]=&clkLED;
    pENB[0]=&enbLED;
    pDIR[0]=&dirLED;
    pSTP[0]=&fulLED;

    pCLK[1]=&clkAPD;
    pENB[1]=&enbAPD;
    pDIR[1]=&dirAPD;
    pSTP[1]=&fulAPD;

    led4=0;
    k=motorID-1;
    if ((motorID==1) ||(motorID==2))
     {
        nOrigin[motorID]=nO;
        nNow[motorID]=nN;
        motorSpd[motorID]=spd;
        if (fullstep==1)
            fullStep[motorID]=1;
        else
            fullStep[motorID]=0;

        *pCLK[k]=0;
        *pENB[k]=0;
        *pDIR[k]=0;
        *pSTP[k]=fullStep[motorID];
        smbed.nNow[motorID]=nNow[motorID];

        return;
    }
    else {
        printf("Wrong motorID while calling setMotor()\n");
    }
}



void moveMotornSteps(int motorID, int nSteps) {
    int nDest;
    nDest=nNow[motorID]+nSteps;
    moveMotor2Dest(motorID, nDest);

}

void moveMotor2Dest(int motorID, int dest) {
    int k;
    DEBUGF("motorID=%d, dest=%d\n",motorID, dest);
    statusLEDMotor=1;

 led1=1;
 led4=1;
    k=motorID-1;
    if ((k==0)||(k==1))
    { // LED or APD motor
        if (*pENB[k]==1) { // motorLEd is moving, have to wait until it stops
            printf("motor[%d] is moving. move cmd is ignored\n", k+1);
            return;
        }
        if (dest==nNow[motorID]) {
        	DEBUGF("motor[%d] arrives at %d \n", motorID, nNow[motorID]);
            dispMotorStatus();
            return;
        }

#if uSWU_POSITION == uSW_OpticRef
        if (uSW==1 && dest>nNow[motorID])
#else
        if (uSW==1 && dest<nNow[motorID])
#endif
	    { // LED motor has been at the end of the rail. No further movement
        	DEBUGF("LED motor[1] has been at the end of the rail. Stop");
        	statusLEDMotor=3;
        	dispMotorStatus();
        	return;
        }
        *pENB[k]=0; // set enbLED=0 to ensure no movement while setting clk
        if (dest<nNow[motorID])
            *pDIR[k]=0;
        else
            *pDIR[k]=1;
        nDest[motorID]=dest;
        led4=0;
        *pCLK[k]=0;
        *pENB[k]=1;
        // tickerLED.attach(&clkMotorLED,0.5/motorSpd[motorID]);
        if (k==0)
           tickerMotor[k].attach(&clkMotorLED,0.5/motorSpd[motorID]);
        else
           tickerMotor[k].attach(&clkMotorAPD,0.5/motorSpd[motorID]);

    } else {
        printf("%d  is an unrecognised MotorID. \n", motorID);
    }
    // DEBUGF("moveMotor2Dest() return\n");

}

// the interrupt routine for tikerMotor[1],
// invoked two times at one step motor movement
void clkMotorLED() {

#if uSWU_POSITION == uSW_OpticRef
	if (uSW==1 && *pDIR[0]==1)
#else
	if (uSW==1 && *pDIR[0]==0)
#endif
	{ // LED motor has been at the end of the rail. No further movement
    	printf("LED motor[1] has been at the end of the rail. \r\n");

        tickerMotor[0].detach();
        *pENB[0]=0;
        led4=0;
        led1=0;
        // dispMotorStatus();
    	statusLEDMotor=3;
        return;
    }

    if ((*pCLK[0]==0) && (nNow[MOTORIDLED]==nDest[MOTORIDLED])) { // stop
        // tickerLED.detach();

        tickerMotor[0].detach();
        *pENB[0]=0;
        led4=0;
        led1=0;
        DEBUGF("motorLED arrives at %d \r\n", nNow[MOTORIDLED]);
        // dispMotorStatus();
        statusLEDMotor=2; //printf("clkMotorLED() set 2\n");
        return;
    }

    *pCLK[0]=!(*pCLK[0]);
     led4=!led4;
    if (*pCLK[0]==0) { // drop edge
        if (*pDIR[0]==0)
            nNow[MOTORIDLED]--;
        else
            nNow[MOTORIDLED]++;
    }

}


// the interrupt routine for tikerLED,
// invoked two times at one step motor movement
void clkMotorAPD() {
int k;
k=1;
    if ((*pCLK[1]==0) && (nNow[MOTORIDAPD]==nDest[MOTORIDAPD])) { // stop
        // tickerLED.detach();
        tickerMotor[1].detach();
        *pENB[k]=0;
        led4=0;
        DEBUGF("motorLED arrives at %d \r\n", nNow[MOTORIDLED]);
        // dispMotorStatus();
        return;
    }

    *pCLK[1]=!(*pCLK[1]);
     led4=!led4;
    if (*pCLK[1]==0) { // drop edge
        if (*pDIR[1]==0)
            nNow[MOTORIDAPD]--;
        else
            nNow[MOTORIDAPD]++;
    }

}
