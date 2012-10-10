#include "mbed.h"
#include "RTCfunc.h"
// #include "SPIA2D.h"

extern void startA2D(unsigned int Fs, unsigned int nSamplesRequired);
extern char ADCstatus;
int nhearton; // for heartbeat
Ticker flipper2;
DigitalOut mheart(LED3);

void dispTime() {

  
  //test sscanf
  /*
  char sentence []="Xuewu Dai's 1 mBed";
  char str [20];
  int i;
  sscanf (sentence,"%s %*s %d",str,&i);
  printf ("\n%s's -> %d-th mBed\n",str,i);
  */
  

    time_t seconds = time(NULL);

   // printf("Time as seconds since January 1, 1970 = %d\n", seconds);

   // printf("Time as a basic string = %s", ctime(&seconds));

    char buffer[32];
    strftime(buffer, 32, "mBed Time: %Y %b %d (%a) %H:%M:%S \n", localtime(&seconds));
    printf("%s\n", buffer);
  
  
}

void setTime(int yy, int mm, int dd, int hh, int min, int ss)
{
    // setup time structure for Wed, 28 Oct 2009 11:35:37
    struct tm t;
    t.tm_sec = ss;    // 0-59
    t.tm_min = min;    // 0-59
    t.tm_hour = hh;   // 0-23
    t.tm_mday = dd;   // 1-31
    t.tm_mon = mm;     // 0-11
    t.tm_year = yy-1900;  // year since 1900

    // convert to timestamp and display (1256729737)
    time_t seconds = mktime(&t);
  //  printf("Time as seconds since January 1, 1970 = %d\n", seconds);
    set_time(seconds);
}

void flip2(){
// flipper2.attach_us(&flip2,1000);
    nhearton=nhearton+1;
    mheart=!mheart;
    if (nhearton>=3)
    { flipper2.detach();
    }
}


void heartbeat() {
    // led3 = !led3;
    // mheart=!mheart; return;
    unsigned int Fs, nSam;
    mheart=1;
    flipper2.attach(&flip2,0.1);
    if (ADCstatus==3)
    {   Fs=1000; nSam=10;
        // printf("\nstart %d A2D@ %d Hz\n", nSam,Fs);
        ADCstatus=0;
        startA2D(Fs,nSam);
    }
    nhearton=0;
}
