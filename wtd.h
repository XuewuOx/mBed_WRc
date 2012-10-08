#include "mbed.h"
// LEDs used to indicate code activity and reset source
// DigitalOut myled1(LED1); //in main loop part 1
// DigitalOut myled2(LED2); //in main loop part 2 (where fault occurs)
// DigitalOut myled3(LED3); //The pushbutton or power on caused a reset
// DigitalOut myled4(LED4); //The watchdog timer caused a reset
 extern DigitalOut led1;
// Simon's Watchdog code from
// http://mbed.org/forum/mbed/topic/508/
class Watchdog {
public:
// Load timeout value in watchdog timer and enable
    void kick(float s) {
        LPC_WDT->WDCLKSEL = 0x1;                // Set CLK src to PCLK
        uint32_t clk = SystemCoreClock / 16;    // WD has a fixed /4 prescaler, PCLK default is /4
        LPC_WDT->WDTC = s * (float)clk;
        LPC_WDT->WDMOD = 0x3;                   // Enabled and Reset
        feed();
    }
// "kick" or "feed" the dog - reset the watchdog timer
// by writing this required bit pattern
    void feed() {
    // led1=!led1;
        LPC_WDT->WDFEED = 0xAA;
        LPC_WDT->WDFEED = 0x55;
    }
};
