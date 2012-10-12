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

#define DEBUG

#ifdef DEBUG
   #define DEBUGF printf
#else
    #define DEBUGF while(0)printf
#endif

void swingLED(int posA, int posB, int nSam);


#endif /* MAIN_H_ */
