#include <stdio.h>
#include <stdlib.h>
#include "MKL46Z4.h"
#include "lcd.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// LED (RG)
// LED_GREEN = PTD5 (pin 98)
// LED_RED = PTE29 (pin 26)

// SWICHES
// RIGHT (SW1) = PTC3 (pin 73)
// LEFT (SW2) = PTC12 (pin 88)

// Enable IRCLK (Internal Reference Clock)
// see Chapter 24 in MCU doc
void irclk_ini()
{
  MCG->C1 = MCG_C1_IRCLKEN(1) | MCG_C1_IREFSTEN(1);
  MCG->C2 = MCG_C2_IRCS(0); //0 32KHZ internal reference clock; 1= 4MHz irc
}

void delay(void)
{
  volatile int i;

  for (i = 0; i < 1000000; i++);
}

// RIGHT_SWITCH (SW1) = PTC3
void sw1_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
  PORTC->PCR[3] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
  GPIOC->PDDR &= ~(1 << 3);
}

// LEFT_SWITCH (SW2) = PTC12
void sw2_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
  PORTC->PCR[12] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
  GPIOC->PDDR &= ~(1 << 12);
}

int sw1_check()
{
  return( !(GPIOC->PDIR & (1 << 3)) );
}

int sw2_check()
{
  return( !(GPIOC->PDIR & (1 << 12)) );
}

// RIGHT_SWITCH (SW1) = PTC3
// LEFT_SWITCH (SW2) = PTC12
void sws_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
  PORTC->PCR[3] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
  PORTC->PCR[12] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
  GPIOC->PDDR &= ~(1 << 3 | 1 << 12);
}

// LED_GREEN = PTD5
void led_green_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
  PORTD->PCR[5] = PORT_PCR_MUX(1);
  GPIOD->PDDR |= (1 << 5);
  GPIOD->PSOR = (1 << 5);
}

void led_green_toggle()
{
  GPIOD->PTOR = (1 << 5);
}

// LED_RED = PTE29
void led_red_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
  PORTE->PCR[29] = PORT_PCR_MUX(1);
  GPIOE->PDDR |= (1 << 29);
  GPIOE->PSOR = (1 << 29);
}

void led_red_toggle(void)
{
  GPIOE->PTOR = (1 << 29);
}

// LED_RED = PTE29
// LED_GREEN = PTD5
void leds_ini()
{
  SIM->COPC = 0;
  SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;
  PORTD->PCR[5] = PORT_PCR_MUX(1);
  PORTE->PCR[29] = PORT_PCR_MUX(1);
  GPIOD->PDDR |= (1 << 5);
  GPIOE->PDDR |= (1 << 29);
  // both LEDS off after init
  GPIOD->PSOR = (1 << 5);
  GPIOE->PSOR = (1 << 29);
}


SemaphoreHandle_t sem;
QueueHandle_t queue;

uint8_t productores = 0;
uint8_t consumidores = 0;
uint8_t mensajes = 0;

void processMessage(int data) {
  delay();  
}

int getData() {
  delay();  
  return rand() % 100; // Datos arbitrarios
}

void prod() {
    int data;
    while (1) {
        if (xSemaphoreTake(sem, portMAX_DELAY) == pdTRUE) {
            for (int i = 0; i < productores; i++) {
                data = getData();
                if (xQueueSend(queue, &data, portMAX_DELAY) == pdTRUE) {
                    mensajes++;
                }
            }
            xSemaphoreGive(sem);
        }
    }
}



void cons() {
    int data;
    while (1) {
        if (xSemaphoreTake(sem, portMAX_DELAY) == pdTRUE) {
            for (int i = 0; i < productores; i++) {
                if (xQueueReceive(queue, &data, portMAX_DELAY) == pdTRUE) {
                    processMessage(data);
                    mensajes--;
                }
            }
            xSemaphoreGive(sem);
        }
    }
}


void lcd_sw(){
  if (sw1_check())
  {   
    productores++;
    if(productores>4){
        productores=0;
    }
    delay();
  }   
  if (sw2_check())
  {   
    if (consumidores>4)
    {
      consumidores=0;
    } 
    consumidores++;
    delay();
  }   

  lcd_set(productores, 1);
  lcd_set(consumidores, 2);
  lcd_set(mensajes, 3);
  lcd_set(mensajes%10, 4);

}

int main(void)
{
  irclk_ini(); // Enable internal ref clk to use by LCD
  lcd_ini();
  sws_ini();
  
  sem = xSemaphoreCreateMutex();
  queue = xQueueCreate(100, sizeof(int));

  xTaskCreate(prod, "Producer", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
  xTaskCreate(cons, "Consumer", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
  xTaskCreate(lcd_sw, "LCD", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

  vTaskStartScheduler();
  
  return 0;
}