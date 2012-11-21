/*
 * SPIA2D.h
 *
 *  Created on: 10 Oct 2012
 *      Author: Xuewu Daniel Dai 
 *       Email: xuewu.dai@eng.ox.ac.uk
 *         URL: http://users.ox.ac.uk/~engs1058/
 *
 */


#ifndef SPIA2D_H_
#define SPIA2D_H_

#define maxChnID 1
#define MAXSAM 2000

void Init_SPIMAX186(void);
void readA2D(char chn);
void startA2D(unsigned int Fs, unsigned int nSamplesRequired);
void stopA2D(void);

#endif /* SPIA2D_H_ */
