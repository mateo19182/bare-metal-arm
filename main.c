#include "MKL46Z4.h"

// LED (RG)
// LED_GREEN = PTD5 (pin 98)
// LED_RED = PTE29 (pin 26)

// SWICHES
// RIGHT (SW1) = PTC3 (pin 73)
// LEFT (SW2) = PTC12 (pin 88)

int state = 1;

void delay(void)
{
  volatile int i;

  for (i = 0; i < 1000000; i++);
}

// RIGHT_SWITCH (SW1) = PTC3
void sw1_ini()
{
  SIM->COPC = 0;  //disables the COP watchdog timer
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;   //enables the clock gate for Port C
  PORTC->PCR[3] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1) | PORT_PCR_IRQC(0b1001);  //PCR for Pin 3 of Port C
  GPIOC->PDDR &= ~(1 << 3); //PDDR to configure Pin 3 of Port C as an input

  NVIC_SetPriority(PORTC_PORTD_IRQn, 0);
  NVIC_EnableIRQ(PORTC_PORTD_IRQn);
}

// LEFT_SWITCH (SW2) = PTC12
void sw2_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
  PORTC->PCR[12] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1) | PORT_PCR_IRQC(0b1001);
  GPIOC->PDDR &= ~(1 << 12);

  NVIC_SetPriority(PORTC_PORTD_IRQn, 0);
  NVIC_EnableIRQ(PORTC_PORTD_IRQn);
}

int sw1_check()
{
  return( !(GPIOC->PDIR & (1 << 3)) );
}

int sw2_check()
{
  return( !(GPIOC->PDIR & (1 << 12)) );
}

void led_green_toggle()
{
  GPIOD->PTOR = (1 << 5);
}

void led_red_toggle(void)
{
  GPIOE->PTOR = (1 << 29);
}

void sw1(void)
{
  switch (state) {
    case 1: //LED vermello acendido e verde apagado,
        led_red_toggle();
        state = 2;
        break;
    case 2: //LED vermello apagado e verde acendido
        led_green_toggle();
        led_red_toggle();
        state = 3;
        break;
    case 3: //dous LEDs acendidos,
        led_red_toggle();
        state = 4;
        break;
    case 4:
        // Turn off both LEDs
        led_green_toggle();
        led_red_toggle();
        state = 1;
        break;
  }
}

void sw2(void) {
  switch (state) {
    case 1:
        led_green_toggle();
        led_red_toggle();
        state = 4;
        break;
    case 2:
        led_green_toggle();
        led_red_toggle();
        state = 3;
        break;
    case 3:
        led_green_toggle();
        led_red_toggle();
        state = 2;
        break;
    case 4:
        led_green_toggle();
        led_red_toggle();
        state = 1;
        break;
  }
}

void PORTDIntHandler() {
    if (PORTC->ISFR & (1 << 3)) {
        sw1();
        PORTC->ISFR = (1 << 3);
    }

    if (PORTC->ISFR & (1 << 12)) {
        sw2();
        PORTC->ISFR = (1 << 12);
    }
}



// LED_RED = PTE29
// LED_GREEN = PTD57

void leds_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;
  PORTD->PCR[5] = PORT_PCR_MUX(1);
  PORTE->PCR[29] = PORT_PCR_MUX(1);
  GPIOD->PDDR |= (1 << 5);
  GPIOE->PDDR |= (1 << 29);
  //LEDS off after init
  GPIOD->PSOR = (1 << 5);
  GPIOE->PSOR = (1 << 29);
}


int main(void) {
    leds_ini();
    sw1_ini();
    sw2_ini();
    
    while (1) {
    }
}
