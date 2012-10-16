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


/* -------------
* external variable declaration
*/
extern MODSERIAL pc;
extern DigitalOut led4;
extern DigitalOut led1;


/* -------------
* variable definition
*/
DigitalOut clkLED(p21);
DigitalOut enbLED(p23);
DigitalOut dirLED(p25); // 0 clockwise, 1 anti-clockwise
// different from Matlab, due to xor in matlab
DigitalOut fulLED(p29); // used to be 27, changed for serial comm

DigitalOut clkAPD(p22);
DigitalOut enbAPD(p24);
DigitalOut dirAPD(p26); // 0 clockwise, 1 anti-clockwise
DigitalOut fulAPD(p30); // used to be 28, changed to 30 for serial comm

DigitalOut *pCLK[2];
DigitalOut *pENB[2];
DigitalOut *pDIR[2];
DigitalOut *pSTP[2];

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
    pc.printf("      move -d|s motorID nStep        : move the motor with motorID \r\n");
    pc.printf("        -d move motor[motorID] to position of nStep \n");
    pc.printf("        -s move motor[motorID] nStep steps. \n");
    pc.printf("          If nStep>0, move forward; if nStep<-0, move backward\r\n");
}

void dispMotorStatus() {
    pc.printf("motor[1] is motorLED: nOrigin=%d, nNow=%d, motorSpd=%3.2f steps/s, fullStep=%d\r\n",nOrigin[1], nNow[1],motorSpd[1],fullStep[1]);

    pc.printf("motor[2] is motorAPD: nOrigin=%d, nNow=%d, motorSpd=%3.2f steps/s, fullStep=%d\r\n",nOrigin[2], nNow[2],motorSpd[2],fullStep[2]);

}

void setMotor(int motorID, int nO, int nN, float spd, int fullstep) {
    int k;

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
            // dispMotorStatus();
            return;
        }
        *pENB[k]=0; // set enbLED=0 to ensure no movement while seting clk
        if (dest<nNow[motorID])
            *pDIR[k]=1;
        else
            *pDIR[k]=0;
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
    if ((*pCLK[0]==0) && (nNow[MOTORIDLED]==nDest[MOTORIDLED])) { // stop
        // tickerLED.detach();
        tickerMotor[0].detach();
        *pENB[0]=0;
        led4=0;
        led1=0;
        DEBUGF("motorLED arrives at %d \r\n", nNow[MOTORIDLED]);
        // dispMotorStatus();
        return;
    }

    *pCLK[0]=!(*pCLK[0]);
     led4=!led4;
    if (*pCLK[0]==0) { // drop edge
        if (*pDIR[0]==1)
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
        if (*pDIR[1]==1)
            nNow[MOTORIDAPD]--;
        else
            nNow[MOTORIDAPD]++;
    }

}
