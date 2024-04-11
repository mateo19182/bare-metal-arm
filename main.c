#include "MKL46Z4.h"
#include "lcd.h"


void delay(void) {
  volatile int i;
  for (i = 0; i < 1000000; i++);
}

void sws_ini() {
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
  PORTC->PCR[3] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
  PORTC->PCR[12] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
  GPIOC->PDDR &= ~(1 << 3 | 1 << 12);
}

int sw1_check() {
  return !(GPIOC->PDIR & (1 << 3));
}

int sw2_check() {
  return !(GPIOC->PDIR & (1 << 12));
}


int main(void) {
  lcd_ini();
  sws_ini();

  uint8_t minutes = 0;
  uint8_t seconds = 0;
  int timerStarted = 0; 

  while (1) {
    // start timer
    if (sw1_check() && sw2_check()) {
      timerStarted = 1;
      delay();
    }

    if (timerStarted) {
      // Placeholder for timer logic to update minutes and seconds

    } else {
      //increment seconds
      if (sw1_check()) {
        seconds++;
        if (seconds > 59) {
          seconds = 0;
          minutes++;
        }
        delay();
      }

      //increment minutes
      if (sw2_check()) {
        minutes++;
        if (minutes > 59) {
          minutes = 0;
        }
        delay();
      }
    }

    // display time
    lcd_display_time(minutes, seconds);

    delay();
  }

  return 0;
}
