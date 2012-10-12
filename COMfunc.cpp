
#include "mbed.h"
#include "COMfunc.h"
#include "main.h"
#include "MODSERIAL.h"
#include "SPIA2D.h"
#include "RTCfunc.h"

#include "stepmotor_ctr.h"

/* ----------------------------------------------
*   External variable defination for COM communication
*/
extern DigitalOut led1;
extern PwmOut led2;
extern DigitalOut led4;

extern float brightness;

// extern void moveMotornSteps(int, int);
// extern void moveMotor2Dest(int, int);
// extern void setMotor(int, int, int, float, int);
// extern void dispMotorStatus(void);

// extern void stopA2D();
// extern void readA2D(char chn);
// extern void startA2D(unsigned int Fs, unsigned int nSamplesRequired);

/* ----------------------------------------------
 *   Variable defination for COM communication
*/
// static char *cmdMoveMotor = "move";

// Serial pc(USBTX,USBRX); // tx, rx
MODSERIAL pc(USBTX,USBRX);

char uartBufIn[UART_BUFFER_SIZE];
char uartBufOut[UART_BUFFER_SIZE];

char msgBufIn[MESSAGE_BUFFER_SIZE];
char msgBufOut[MESSAGE_BUFFER_SIZE];
int endofcmd=-1; // -1 for normal char, 0 for 0x0D, 1 for 0x0A
                 // endofcmd =1 (TRUE) only when received 0D 0A ("\r\n")
int nCharIn;
int nCharOut;

int nMsgCharIn;



// -----------------------
//  variables and functions of UART(P13,P14) for PIC-SineGnerator

 MODSERIAL irdrive(p9,p10);
 // MODSERIAL sinegen(p13,p14);
 MODSERIAL uvdrive(p28,p27);

void initCOMpc();
void Tx13_interrupt(MODSERIAL_IRQ_INFO *q);
void Rx14_interrupt(MODSERIAL_IRQ_INFO *q);

void Tx28_interrupt(MODSERIAL_IRQ_INFO *q);
void Rx27_interrupt(MODSERIAL_IRQ_INFO *q);

void txCallback(MODSERIAL_IRQ_INFO *q);
void txEmpty(MODSERIAL_IRQ_INFO *q);
void rxCallback(MODSERIAL_IRQ_INFO *q);
bool strcmp2(char *str1, char *str2, int len);


// Circular buffers for serial TX and RX data - used by interrupt routines
const int buffer_size = 255;
// might need to increase buffer size for high baud rates
char tx13_buffer[buffer_size];
char rx14_buffer[buffer_size];
// Circular buffer pointers
// volatile makes read-modify-write atomic
volatile int tx13_in=0;
volatile int tx13_out=0;

//------------------------

//----------------------------------------------
/*
*   Function defination for COM communication
*/

void initCOMpc()
{
    irdrive.baud(9600);
    // Setup a serial interrupt function to receive data
    irdrive.attach(&Rx14_interrupt, MODSERIAL::RxIrq);
    // Setup a serial interrupt function to transmit data
    irdrive.attach(&Tx13_interrupt, MODSERIAL::TxIrq);
    
    uvdrive.baud(9600);
    uvdrive.attach(&Rx27_interrupt, MODSERIAL::RxIrq);
    uvdrive.attach(&Tx28_interrupt, MODSERIAL::TxIrq);
    
  //  sinegen.baud(9600);
  //  sinegen.attach(&Rx27_interrupt, MODSERIAL::RxIrq);
  //  sinegen.attach(&Tx28_interrupt, MODSERIAL::TxIrq);

    pc.baud(115200);
//   pc.attach(&rxCallback, Seriall::RxIrq);
//    pc.attach(&rxCallback);
    pc.attach(&txCallback, MODSERIAL::TxIrq);
    pc.attach(&rxCallback, MODSERIAL::RxIrq);
    pc.attach(&txEmpty,    MODSERIAL::TxEmpty);
    nCharIn=0;
}


void txCallback(MODSERIAL_IRQ_INFO *q) {
   // led1 = !led1;
    // wait(0.1);
}

// This function is called when TX buffer goes empty
// only being called when sending by puts() or printf();
//      putc() does not triger the inttrupt of txEmpty
void txEmpty(MODSERIAL_IRQ_INFO *q) {
    led1 = 0;
  //  char c='E';
 //   pc.putc(c);
   //  pc.puts(" Done. ");
}


// This function is called when a character goes into the RX buffer.
void rxCallback(MODSERIAL_IRQ_INFO *q) {
    // led1 = !led1;
    int i;
    // if (pc.readable()) {
    char c=pc.getc();
    uartBufIn[nCharIn++]=c;
    // pc.putc(c);
    if (c=='\r')
    { endofcmd=0;
      return;
    }
    if (endofcmd==0 && c=='\n')
    {   
        for (i=0;i<nCharIn;i++)
           msgBufIn[i]=uartBufIn[i];
        msgBufIn[i]=0; // terminate the string
        nMsgCharIn=nCharIn;
        nCharIn=0; // set nCharIn=0 to clear uartBufIn
        endofcmd=1;
        return;
    }
    
    endofcmd=-1;
    if ((c == '^') && (brightness < 0.5)) {
                brightness += 0.01;
                led2 = brightness;
            }
            if ((c == 'V') && (brightness > 0.0)) {
                brightness -= 0.01;
                led2 = brightness;
            }
    // pc.puts("Done.");
    // } // end of if (pc.readable())
    
}

void cmdProcess()
{ int nValidArgs;
  int k,motorID;
  unsigned int Fs, nSam;
  char moveType;
  char chID; // ID of select A2D channel for AD conversion
  int nOri, nNow, fullstep;
  float spd;

  int aa, bb;
  // printf("received cmd %s",msgBufIn);
  moveType='s';
  k=0;
  motorID=1;
  // DEBUGF("msgBuffIn = %s\n", msgBufIn);
  if (strcmp2(msgBufIn,"resetmbed",9)==1)
  {
	  printf("\n Got it. mBed will reset in 10 sec \n");
	  while(1);
  }

  if (strcmp2(msgBufIn,"move",4)==1)
  {
    nValidArgs=sscanf(msgBufIn, "move -%c %d %d\n", &moveType, &motorID, &k);
    DEBUGF("%d Args: moveType=%c, ID=%d, para=%d\n", nValidArgs,moveType, motorID, k);
    if (nValidArgs!=3)
    { // wrong command
     printf("No enought input parameters. move -s|d motorID int\n");
     return;
    }
    if (moveType=='d')
        moveMotor2Dest(motorID,k);
    else if (moveType=='s')
        moveMotornSteps(motorID, k);
    else
        printf("unrecognised command. command format: move -d|s motorID int \r\n");
    return;
   }
   
  if (strcmp2(msgBufIn,"setm",4)==1)
  {  nValidArgs=sscanf(msgBufIn, "setm %d %d %d %f %d\n", &motorID, &nOri, &nNow, &spd, &fullstep);
    if (nValidArgs < 5)
    { // wrong command
     printf("No enought input parameters. setm motorID nOrigin nNow speed fullstep\n");
     return;
    }
   if (nValidArgs > 5)
    { // wrong command
     printf("Too many input parameters. setm motorID nOrigin nNow speed fullstep\n");
     return;
    }
    // correct number of input arguments
    setMotor(motorID, nOri, nNow, spd, fullstep);
    dispMotorStatus();
     return;
  }
  
   if (strcmp2(msgBufIn,"ir",2)==1)
   {
     // DEBUGF("irdrive cmd detected, %s\n", msgBufIn);
    // Start Critical Section - don't interrupt while changing global buffer variables
    nValidArgs=sscanf(msgBufIn, "ir%s\n", tx13_buffer);
    irdrive.puts(tx13_buffer);
    pc.puts(tx13_buffer);
    led4=!led4;
    return;
   }
   
   if (strcmp2(msgBufIn,"uv",2)==1)
      {
        // DEBUGF("irdrive cmd detected, %s\n", msgBufIn);
       // Start Critical Section - don't interrupt while changing global buffer variables
       nValidArgs=sscanf(msgBufIn, "uv%s\n", tx13_buffer);
       uvdrive.puts(tx13_buffer);
       irdrive.puts(tx13_buffer);
       pc.puts(tx13_buffer);
       led4=!led4;
       return;
      }


   if (strcmp2(msgBufIn,"a2d",3)==1)
   {
     // DEBUGF("irdrive cmd detected, %s\n", msgBufIn);
    // Start Critical Section - don't interrupt while changing global buffer variables
    nValidArgs=sscanf(msgBufIn, "a2d %c %d %d\n", &chID, &Fs, &nSam);
    // DEBUGF("start %d A2D conversion at %dHz.\n", nSam,Fs);
    if (chID=='s')
    { // start continous AD conversion until a2d
        if (nValidArgs==3)
            {
             DEBUGF("start %d A2D conversion at %dHz. ", nSam,Fs);
             dispTime(); 
             startA2D(Fs,nSam);
             }
        else
          DEBUGF("Un-correct a2d command, ignored. \n");
    }
    else if(chID=='c')
    { // stop A2D
        stopA2D();
        DEBUGF("Stop A2D...Done!"); dispTime(); 
    }
    else if (chID>='0' && chID<'8')
         readA2D(chID-0x30);
         
    led4=!led4;
    return;
   }


   if (strcmp2(msgBufIn,"swn",3)==1)
   { // swing LED source and collect UV/IR data
	  nValidArgs=sscanf(msgBufIn, "swn %d %d %d\n", &aa, &bb, &nSam);
	  if (nValidArgs==3)
	  {
		  swingLED(aa, bb, nSam);
	   }
	  else
	  {
		  DEBUGF("uncompleted command, ignored. \n");
	  }
	  DEBUGF("cmdPrcess() returns\n");
	  return;
   }
  // not a pre-defined command, ignored.
  DEBUGF("Un-recognised command, ignored. \n");
}


// Interupt Routine to read in data from serial port
void Rx14_interrupt(MODSERIAL_IRQ_INFO *q) {
    char rxch;
    led4=!led4;
// Loop just in case more than one character is in UART's receive FIFO buffer
// Stop if buffer full
    while (irdrive.readable()) {
        rxch = irdrive.getc();
// Uncomment to Echo to USB serial to watch data flow
        pc.putc(rxch);
        
        }
    led4=0;
    return;
}

// Interupt Routine to write out data to serial port
void Tx13_interrupt(MODSERIAL_IRQ_INFO *q) {
  //  led4debug=1;
  led1=!led1;
// Loop to fill more than one character in UART's transmit FIFO buffer
// Stop if buffer empty
    while ( tx13_out !=tx13_in) {
        irdrive.putc(tx13_buffer[tx13_out]);
        // pc.putc(tx13_buffer[tx13_out]);
        DEBUGF("%c",tx13_buffer[tx13_out]);
        tx13_out = (tx13_out + 1) % buffer_size;
    }
   // led4debug=0;
   // DEBUGF("%c",tx13_buffer[tx13_out]);
    return;
}


// Interupt Routine to read in data from serial port
void Rx27_interrupt(MODSERIAL_IRQ_INFO *q) {
    char rxch;
    led4=!led4;
// Loop just in case more than one character is in UART's receive FIFO buffer
// Stop if buffer full
    while (uvdrive.readable()) {
        rxch = uvdrive.getc();
// Uncomment to Echo to USB serial to watch data flow
        pc.putc(rxch);
        }
    led4=0;
    return;
}
// Interupt Routine to write out data to serial port
void Tx28_interrupt(MODSERIAL_IRQ_INFO *q) {
  //  led4debug=1;
  led1=!led1;
// Loop to fill more than one character in UART's transmit FIFO buffer
// Stop if buffer empty
    while ( tx13_out !=tx13_in) {
        uvdrive.putc(tx13_buffer[tx13_out]);
        // pc.putc(tx13_buffer[tx13_out]);
        DEBUGF("%c",tx13_buffer[tx13_out]);
        tx13_out = (tx13_out + 1) % buffer_size;
    }
   // led4debug=0;
   // DEBUGF("%c",tx13_buffer[tx13_out]);
    return;
}

bool strcmp2(char *str1, char *str2, int len)
{
    int i;
    bool same;
    same=1;
    for (i=0; i<len; i++)
    { 
    // printf("i=%i,str1=%c, str2=%c, same=%d\n",i,str1[i], str2[i],same);
    if (str1[i]!=str2[i])
       { same=0;
         break;
         }
    }
    return same;
}
