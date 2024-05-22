/*
 * The Clear BSD License
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "MKL46Z4.h"
#include "lcd.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "fsl_gpio.h"


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

void led_green_toggle()
{
    GPIOD->PTOR = (1 << 5);
}

void led_red_toggle(void)
{
    GPIOE->PTOR = (1 << 29);
}

int is_led1_off() {
    return (GPIOD->PDIR & (1 << 5)) != 0;
}

int is_led2_off() {
    return (GPIOE->PDIR & (1 << 29)) != 0;
}

void sw1_ini()
{
  SIM->COPC = 0;  //disables the COP watchdog timer
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;   //enables the clock gate for Port C
  PORTC->PCR[3] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1) | PORT_PCR_IRQC(0b1001);  //PCR for Pin 3 of Port C
  GPIOC->PDDR &= ~(1 << 3); //PDDR to configure Pin 3 of Port C as an input

  NVIC_SetPriority(PORTC_PORTD_IRQn, 0);
  NVIC_EnableIRQ(PORTC_PORTD_IRQn);
}

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

void sw1(void)
{ 
    // acende, se está apagado, o LED da esquerda, apagando o outro se estaba acendido.  
    if (is_led1_off())
    {
        led_green_toggle();
    }
    if (!(is_led2_off()))
    {
        led_red_toggle();
    }

}

void sw2(void)
{
    if (is_led2_off())
    {
        led_red_toggle();
    }
    if (!(is_led1_off()))
    {
        led_green_toggle();
    }
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

int is_digit(char c) {
    return c >= '0' && c <= '9';
}

int is_valid_expression(const char *str) {
    int seen_digit = 0; // Flag to ensure there's at least one digit
    if (!*str) // Empty string is not a valid expression
        return 0;
    while (*str) {
        // Cast *str to unsigned char to avoid potential negative char issues
        if (is_digit((unsigned char)*str)) { // Check if the character is a digit
            seen_digit = 1;  // Mark that we've seen at least one digit
        } else {
            // Allow '+' or '-' only if they follow a digit and are not the last character
            if ((*str != '+' && *str != '-') || !seen_digit || *(str + 1) == '\0') {
                return 0;
            }
            seen_digit = 0; 
        }
        str++;
    }
    return seen_digit;
}

int evaluate_expression(const char *expr) {
    int result = 0;     // Holds the final result
    int current_number = 0; // Temporary storage for the current number being parsed
    int sign = 1;       // Sign of the current number, 1 for positive, -1 for negative

    while (*expr) {
        if (is_digit(*expr)) {
            // Build the current number digit by digit
            current_number = current_number * 10 + (*expr - '0');
        } else if (*expr == '+' || *expr == '-') {
            // When we hit an operator, we apply the previous number and reset for the next
            result += sign * current_number;
            current_number = 0;
            sign = (*expr == '-') ? -1 : 1; // Update the sign based on the operator
        }
        expr++;
    }
    // Add the last number parsed (if any)
    result += sign * current_number;

    return result;
}

int main(void)
{
    irclk_ini(); // Enable internal ref clk to use by LCD
    lcd_ini();    

    sw1_ini();
    sw2_ini();
    leds_ini();

    char command[100];
    int index = 0;

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();


    PRINTF("\r\nReinicio!\r\n$ ");

    while (1) {
        char ch = GETCHAR();
        if (ch == '\r') {
            command[index] = '\0'; // Finaliza a cadea
            if (strcmp(command, "led1") == 0) {
                led_green_toggle();
            } else if (strcmp(command, "led2") == 0) {
                led_red_toggle();
            } else if (strcmp(command, "off") == 0) {
                GPIOD->PSOR = (1 << 5);
                GPIOE->PSOR = (1 << 29);
            } else if (strcmp(command, "toggle") == 0) {
                led_red_toggle();
                led_green_toggle();
            }
            else if (is_valid_expression(command) ) {
                int res = evaluate_expression(command);                
                lcd_display_dec2(res);

                // for (int i = 0; command[i] != '\0'; i++) {
                //     PUTCHAR('\n');
                //     PUTCHAR(command[i]);
                // }
                
            }
            PUTCHAR('\r'); // Ensure carriage return
            PUTCHAR('\n'); // Move to the next line
            index = 0;
            PRINTF("$ ");
        } else {
            PUTCHAR(ch);
            command[index++] = ch;
        }
    }
}

