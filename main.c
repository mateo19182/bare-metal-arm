
#include "MKL46Z4.h"
#include "lcd.h"
#include "fsl_tpm.h"

#define TPM_SOURCE_CLOCK (CLOCK_GetFreq(kCLOCK_PllFllSelClk)/4)
#define BOARD_TPM TPM0
#define BOARD_TPM_IRQ_NUM TPM0_IRQn
#define BOARD_TPM_HANDLER FTM0IntHandler
#define TPM_PRESCALER kTPM_Prescale_Divide_4

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

int main(void)
{
    irclk_ini();
    sws_ini();
    lcd_ini();

    uint8_t minutes = 0;
    uint8_t seconds = 0;
    int timerStarted = 0;

    lcd_display_time(minutes, seconds);

    tpm_config_t tpmInfo;

    //delay();

    //BOARD_InitPins();
    //BOARD_BootClockRUN();
    
    CLOCK_SetTpmClock(3);
    //SIM-> SCGC6 |= SIM_SCGC6_TMP(1);
    //SIM-> SOPT2 |= SIM_SOPT2_TMPSCR(3);

    TPM_GetDefaultConfig(&tpmInfo);

    //initialize TPM with the config
    TPM_Init(BOARD_TPM, &tpmInfo);

    // timer period = 1 second
    TPM_SetTimerPeriod(BOARD_TPM, USEC_TO_COUNT(10000U, TPM_SOURCE_CLOCK));

    // enable Timer Overflow Interrupt
    TPM_EnableInterrupts(BOARD_TPM, kTPM_TimeOverflowInterruptEnable);

    // enable IRQ for TPM
    EnableIRQ(BOARD_TPM_IRQ_NUM);

    // start timer
    //TPM_StartTimer(BOARD_TPM, kTPM_SystemClock);

    while (1)
    {
        // start timer
        if (sw1_check() && sw2_check())
        {
            if (timerStarted)
            {
                timerStarted = 0;
                TPM_StopTimer(BOARD_TPM);
            }
            else
            {
                timerStarted = 1;
                TPM_StartTimer(BOARD_TPM, kTPM_SystemClock);
            }
            //minutes = 0;
            //seconds = 0;
            delay();
        }
        // timer logic
        if (timerStarted)
        {
            while (true)
            {
                if (tpmIsrFlag)
                {
                    tpmIsrFlag = false;
                    seconds--;
                    if (seconds == 0)
                    {                        
                        if (minutes == 0)                            
                        {
                            TPM_StopTimer(BOARD_TPM);
                            timerStarted = 0;
                            break;
                        }
                        else
                        {
                            seconds = 59;
                            minutes--;
                        }
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

void FTM0IntHandler(void)
{
    // clear interrupt flag
    TPM_ClearStatusFlags(BOARD_TPM, kTPM_TimeOverflowFlag);
    tpmIsrFlag = true;
}