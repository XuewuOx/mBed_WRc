/*
 * stepmotor_ctr.h
 *
 *  Created on: 12 Oct 2012
 *      Author: Xuewu Daniel Dai 
 *       Email: xuewu.dai@eng.ox.ac.uk
 *         URL: http://users.ox.ac.uk/~engs1058/
 *
 */


#ifndef STEPMOTOR_CTR_H_
#define STEPMOTOR_CTR_H_


void dispMotorCmdHelp(void);
void clkMotorLED(void);
void clkMotorAPD(void);
void dispMotorStatus(void);
void moveMotor2Dest(int motorID, int dest);
void moveMotornSteps(int motorID, int nSteps);
void setMotor(int motorID, int nO, int nN, float spd, int fullstep);



#define NUMMOTOR 3

#define MOTORIDLED 1
#define MOTORIDAPD 2

#define FULLSTEP 1
#define HALFSTEP 0


#endif /* STEPMOTOR_CTR_H_ */
