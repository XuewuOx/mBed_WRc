 #define DEBUG

#ifdef DEBUG
   #define DEBUGF printf
#else
    #define DEBUGF while(0)printf
#endif