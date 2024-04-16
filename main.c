#include "MKL46Z4.h"
#include "lcd.h"
#include "fsl_tpm.h"

#define BOARD_TPM TPM0
#define TPM_SOURCE_CLOCK (CLOCK_GetFreq(kCLOCK_PllFllSelClk) / 4)
#define BOARD_TPM_IRQ_NUM TPM0_IRQn

volatile bool tpmIsrFlag = false;
volatile uint32_t milisecondCounts = 0U;

void irclk_ini()
{
    MCG->C1 = MCG_C1_IRCLKEN(1) | MCG_C1_IREFSTEN(1);
    MCG->C2 = MCG_C2_IRCS(0);
}

void delay(void)
{
    volatile int i;
    for (i = 0; i < 100000; i++)
        ;
}

void sws_ini()
{
    SIM->COPC = 0;
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
    PORTC->PCR[3] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
    PORTC->PCR[12] |= PORT_PCR_MUX(1) | PORT_PCR_PE(1);
    GPIOC->PDDR &= ~(1 << 3 | 1 << 12);
}

int sw1_check()
{
    return !(GPIOC->PDIR & (1 << 3));
}

int sw2_check()
{
    return !(GPIOC->PDIR & (1 << 12));
}

//MCGIntHandler
void TPM0_IRQHandler(void)
{
    // Clear interrupt flag
    TPM_ClearStatusFlags(BOARD_TPM, kTPM_TimeOverflowFlag);
    tpmIsrFlag = true;
}

int main(void)
{
    irclk_ini();
    lcd_ini();
    sws_ini();

    uint8_t minutes = 0;
    uint8_t seconds = 0;
    int timerStarted = 0;
    tpm_config_t tpmInfo;

    TPM_GetDefaultConfig(&tpmInfo);

    // Modify the TPM settings if needed, then initialize TPM with the config
    TPM_Init(BOARD_TPM, &tpmInfo);

    // Set timer period for 1 second
    TPM_SetTimerPeriod(BOARD_TPM, USEC_TO_COUNT(1000000U, TPM_SOURCE_CLOCK));

    // Enable Timer Overflow Interrupt
    TPM_EnableInterrupts(BOARD_TPM, kTPM_TimeOverflowInterruptEnable);

    // Enable the IRQ for the TPM
    EnableIRQ(BOARD_TPM_IRQ_NUM);

    // Start the timer
    TPM_StartTimer(BOARD_TPM, kTPM_SystemClock);

    while (1)
    {
        // start timer
        if (sw1_check() && sw2_check())
        {
            timerStarted = 1;
            minutes = 0;
            seconds = 0;
            delay();
        }

        // timer logic
        if (timerStarted)
        {
            while (1)
            {
                if (tpmIsrFlag)
                {
                    tpmIsrFlag = false;
                    seconds++;
                    if (seconds >= 60)
                    {
                        seconds = 0;
                        minutes++;
                    }
                    if (minutes >= 60)
                    {
                        minutes = 0;
                    }
                    lcd_display_time(minutes, seconds);
                }
            }
        }
        else
        {
            // increment seconds
            if (sw1_check())
            {
                seconds++;
                if (seconds > 59)
                {
                    seconds = 0;
                    minutes++;
                }
                delay();
            }

            // increment minutes
            if (sw2_check())
            {
                minutes++;
                if (minutes > 59)
                {
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
