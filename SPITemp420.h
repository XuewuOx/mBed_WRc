/*
 * SPITemp420.h
 *
 *  Created on: 21 Feb 2013
 *      Author: Xuewu Daniel Dai 
 *       Email: xuewu.dai@eng.ox.ac.uk
 *         URL: http://users.ox.ac.uk/~engs1058/
 * %              _
 * %  \/\ /\ /   /  * _  '
 * % _/\ \/\/ __/__.'(_|_|_
 */


#ifndef SPITEMP420_H_
#define SPITEMP420_H_

void Init_SPITemp420();
float readTemp();
void startReadingTemp(float rdInterval_s);
int set420mAOutput(float curr, int chID);

#endif /* SPITEMP420_H_ */
