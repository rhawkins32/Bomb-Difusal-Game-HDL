 #include "msp.h"

/* S -> Segment on 7 Segment Display
 *         A
 *       F   B
 *         G
 *       E   C
 *         D   DOT
 * */
#define SA BIT0
#define SB BIT1
#define SC BIT2
#define SD BIT3
#define SE BIT4
#define SF BIT5
#define SG BIT6
#define SDOT BIT7

#define TIMER_CYCLE 1500000
#define LED1 BIT6
#define LED2 BIT2
#define LED3 BIT0
#define LED4 BIT7
#define LED5 BIT6
int defeatValue = 0;
int victoryValue = 0;
int timerStage = 5;
int rand = -1;
int Tick = 1;
int sequence = 1;

void setUpSevenSeg(int number);
int numberToSevenSeg(int number);
void displaySevenSeg(int digit0, int digit1, int digit2, int digit3);

void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer


	/*Input setup*/
	/*6.0-6.5*/
	P6->DIR &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5); /* set pins as input */
	P6->IES |= (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5); /* Set edge to "Falling Edge" */
	P6->IE |= (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5); /* Enable the interrupt for each bit */
	P6->REN |= (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5);

	/*Reset Switch setup*/
	P1->DIR &= ~BIT1; /*S1 as input*/
	P1->REN |= BIT1;
	P1->IES |= BIT1;
	P1->IE |= BIT1;
	P1->OUT |= BIT1;
	P1->IFG = 0;

	/* LED setup */
	/* Test LED */
	P1->DIR |= BIT0;
	P1->OUT &= ~BIT0;
	/* P3.6, P5.2, P5.0, P1.7, P1.6 Are LEDs */
	P3->DIR |= LED1;/* set LED1 as output */
	P3->OUT |= LED1;/* turn on LED1 */

    P5->DIR |= LED2;/* set LED2 as output */
    P5->OUT |= LED2;/* turn on LED2 */

    P5->DIR |= LED3;/* set LED3 as output */
    P5->OUT |= LED3;/* turn on LED3 */

    P1->DIR |= LED4;/* set LED4 as output */
    P1->OUT |= LED4;/* turn on LED4 */

    P1->DIR |= LED5;/* set LED5 as output */
    P1->OUT |= LED5;/* turn on LED5 */

    /* Seven Segment Setup */
    P2->DIR = SA | SB | SC | SD | SE | SF| SG | SDOT; /* set Seven Segment to output */
    P4->DIR |= (BIT0 | BIT1 | BIT2 |BIT3); /* Digit select pins */

    /* Speaker Setup */
    P3->OUT &= ~BIT0;/* turn off Speaker */
    P3->DIR |= BIT0; /* set Speaker as output */

	/* Clock System Setup*/
	CS->KEY = CS_KEY_VAL;
	CS->CTL1 = CS_CTL1_SELM_3;/* Set the REFO frequency to 32 kHz */
	/*CS->CTL1 |= CS_CTL1_DIVM_1; Initialize the Master Clock to the REFO source with a divider of 2 */

	/* Power Control Module */
	PCM->CTL0 = PCM_CTL0_KEY_VAL | PCM_CTL0_AMR_8;/* Active mode, low frequency, and Vcore level 0 */

    /* set up the Timer32 module 0 with the frequency divider at 16, the size at 32 bits, and the mode set to periodic */
	TIMER32_1->CONTROL = TIMER32_CONTROL_MODE;
	TIMER32_1->CONTROL |= TIMER32_CONTROL_PRESCALE_1 | TIMER32_CONTROL_SIZE;
	TIMER32_1->LOAD |= TIMER_CYCLE;/* set the initial count of Timer32 module 0 to the TIMER_PERIOD constant */
	TIMER32_1->INTCLR = 1;


	/* Enable interrupts and sleep on exit */
    /* TODO: enable interrupt for pins that are pulled */
	NVIC->ISER[0] |= 0x02000000;/* enable the Timer32 interrupts */
	NVIC->ISER[1] |= 0x00000100;/* enable the Port 6 interrupts */
	NVIC->ISER[1] |= 0x00000008;/* enable S1 interrupt */
	_enable_interrupts();/* enable the master interrupt */
	__enable_irq();

	/* P4->IE |= BIT0; enable the interrupt for pin P4.0  TODO: enable interrupt for pins that are pulled */
	TIMER32_1->CONTROL |= TIMER32_CONTROL_IE;/* enable the interrupt for Timer32 module 0 */
	TIMER32_1->CONTROL |= TIMER32_CONTROL_ENABLE; /* enable the TIMER32 Clock */
	while(1)
	{
	    int digit1 = (defeatValue * 100) + victoryValue;
        setUpSevenSeg(digit1);
	};
}

/* SEV SEGMENT FUNCTIONS */

/* Delay */

void delay(uint32_t delay_ms) // assumes 1 ms tick.
{
    uint32_t start = Tick;

    while (start < delay_ms)
    {
        start += Tick;
    };
}

 /*
 * NUMBER TO SEVEN SEG
 * Converts a Inputted Value into a value that can be displayed on a seven segment display
 */
int numberToSevenSeg(int number)
{
    int value = 0;
    switch(number)
    {
                    case 0:
                        value = SA | SB | SC | SD | SE | SF;
                        break;

                    case 1:
                        value = SB | SC;
                        break;

                    case 2:
                        value = SA | SB | SD | SE | SG;
                        break;

                    case 3:
                        value = SA | SB | SC | SD | SG;
                        break;

                    case 4:
                        value = SB | SC | SF | SG;
                        break;

                    case 5:
                        value = SA | SC | SD | SF | SG;
                        break;

                    case 6:
                        value = SA | SC | SD | SE | SF | SG;
                        break;

                    case 7:
                        value = SA | SB | SC;
                        break;

                    case 8:
                        value = SA | SB | SC | SD | SE | SF| SG;
                        break;

                    case 9:
                        value =  SA | SB | SC | SF | SG;
                        break;

                    default:
                        value = SDOT;
    }
    return value;

}

/*
 * DISPLY SEVEN SEG
 */
void displaySevenSeg(int digit0, int digit1, int digit2, int digit3)
{

    P4->OUT = ~BIT0;
    P2->OUT = numberToSevenSeg(digit0);
    delay(100);

    P4->OUT = ~BIT1;
    P2->OUT = numberToSevenSeg(digit1);
    delay(100);

    P4->OUT = ~BIT2;
    P2->OUT = numberToSevenSeg(digit2);
    delay(100);

    P4->OUT = ~BIT3;
    P2->OUT = numberToSevenSeg(digit3);
    delay(100);
}

void setUpSevenSeg(int number)
{

    int digit4 = -1;
    int digit3 = -1;
    int digit2 = -1;
    int digit1 = -1;

    if(number <= 9999 && number >= 0);
    {

       digit4 = number / 1000;
       number = number - (digit4 * 1000);
       digit3 = number / 100;
       number = number - (digit3 * 100);
       digit2 = number / 10;
       number = number - (digit2 * 10);
       digit1 = number;
    }


    displaySevenSeg(digit1, digit2, digit3, digit4);

}

/* Other functions */

void seriesFlash(void)
{
    P3->OUT &= ~LED1;/* turn off LED1 */
    P5->OUT &= ~LED2;/* turn off LED2 */
    P5->OUT &= ~LED3;/* turn off LED3 */
    P1->OUT &= ~LED4;/* turn off LED4 */
    P1->OUT &= ~LED5;/* turn off LED5 */
    delay(0x04000);
    P3->OUT |= LED1;/* turn on LED1 */
    delay(0x04000);
    P3->OUT &= ~LED1;/* turn off LED1 */
    P5->OUT |= LED2;/* turn on LED2 */
    delay(0x04000);
    P5->OUT &= ~LED2;/* turn off LED2 */
    P5->OUT |= LED3;/* turn on LED3 */
    delay(0x04000);
    P5->OUT &= ~LED3;/* turn off LED3 */
    P1->OUT |= LED4;/* turn on LED4 */
    delay(0x04000);
    P1->OUT &= ~LED4;/* turn off LED4 */
    P1->OUT |= LED5;/* turn on LED5 */
    delay(0x04000);
    P1->OUT &= ~LED5;/* turn off LED5 */
}
void flashLightOnce(void)
{
    P3->OUT |= LED1;/* turn on LED1 */
    P5->OUT |= LED2;/* turn on LED2 */
    P5->OUT |= LED3;/* turn on LED3 */
    P1->OUT |= LED4;/* turn on LED4 */
    P1->OUT |= LED5;/* turn on LED5 */
    delay(0x15000);
    P3->OUT &= ~LED1;/* turn off LED1 */
    P5->OUT &= ~LED2;/* turn off LED2 */
    P5->OUT &= ~LED3;/* turn off LED3 */
    P1->OUT &= ~LED4;/* turn off LED4 */
    P1->OUT &= ~LED5;/* turn off LED5 */
    delay(0x15000);
}
void flashLightLong(void)
{
    P3->OUT |= LED1;/* turn on LED1 */
    P5->OUT |= LED2;/* turn on LED2 */
    P5->OUT |= LED3;/* turn on LED3 */
    P1->OUT |= LED4;/* turn on LED4 */
    P1->OUT |= LED5;/* turn on LED5 */
    delay(0x50000);
    P3->OUT &= ~LED1;/* turn off LED1 */
    P5->OUT &= ~LED2;/* turn off LED2 */
    P5->OUT &= ~LED3;/* turn off LED3 */
    P1->OUT &= ~LED4;/* turn off LED4 */
    P1->OUT &= ~LED5;/* turn off LED5 */
}

void victorySequence(void)
{
    P3->OUT |= BIT0;/* turn on Speaker */
    delay(0x03000);
    P3->OUT &= !BIT0;/* turn off Speaker */
    delay(0x03000);
    P3->OUT |= BIT0;/* turn on Speaker */
    delay(0x03000);
    P3->OUT &= !BIT0;/* turn off Speaker */
    seriesFlash();
    seriesFlash();
    seriesFlash();
    seriesFlash();
    seriesFlash();
    TIMER32_1->CONTROL &= ~TIMER32_CONTROL_ENABLE; /* Disable the clock */
    victoryValue++;
}
void defeatSequence(void)
{
    P3->OUT |= BIT0;/* turn on Speaker */
    flashLightOnce();
    flashLightOnce();
    flashLightLong();
    P3->OUT &= !BIT0;/* turn off Speaker */
    TIMER32_1->CONTROL &= ~TIMER32_CONTROL_ENABLE; /* Disable the clock */
    defeatValue++;
}

/* Bomb defusal*/
void PORT6_IRQHandler(void)
{
    P6->IFG = 0;
    /* Generate a random number by taking the current value of the clock when a wire is pulled. */
    if (rand == -1)
        {rand = (TIMER32_1->VALUE & 0x00000100);}
    if (rand)
    {
        /* Scenario 1 */
        if(((P6->IN & BIT0) == 0) && ((P6->IE & BIT0) == BIT0))
        {
            P1->OUT ^= BIT0;
            P6->IE &= ~BIT0;
        }
        if(((P6->IN & BIT1) == 0) && ((P6->IE & BIT1) == BIT1))
        {
            P1->OUT ^= BIT0;
            P6->IE &= ~BIT1;
        }
        if(((P6->IN & BIT2) == 0) && ((P6->IE & BIT2) == BIT2))
        {
            P1->OUT ^= BIT0;
            P6->IE &= ~BIT2;
        }
        if(((P6->IN & BIT3) == 0) && ((P6->IE & BIT3) == BIT3))
        {
            P1->OUT ^= BIT0;
            P6->IE &= ~BIT3;
        }
        if(((P6->IN & BIT4) == 0) && ((P6->IE & BIT4) == BIT4))
        {
            P1->OUT ^= BIT0;
            P6->IE &= ~BIT4;
            defeatSequence();
        }
        if(((P6->IN & BIT5) == 0) && ((P6->IE & BIT5) == BIT5))
        {
            P1->OUT ^= BIT0;
            P6->IE &= ~BIT5;
        }
        if(P6->IE == BIT4)
        {
            if(sequence)
            {
               victorySequence();
               sequence = 0;
            }
        }
    }
    else if (rand == 0)
    {
        /* Scenario 2 */
        if(((P6->IN & BIT0) == 0) && ((P6->IE & BIT0) == BIT0))
        {
            P1->OUT ^= BIT0;
            P6->IE &= ~BIT0;
        }
        if(((P6->IN & BIT1) == 0) && ((P6->IE & BIT1) == BIT1))
        {
            P1->OUT ^= BIT0;
            P6->IE &= ~BIT1;
            defeatSequence();
        }
        if(((P6->IN & BIT2) == 0) && ((P6->IE & BIT2) == BIT2))
        {
            P1->OUT ^= BIT0;
            P6->IE &= ~BIT2;
        }
        if(((P6->IN & BIT3) == 0) && ((P6->IE & BIT3) == BIT3))
        {
            P1->OUT ^= BIT0;
            P6->IE &= ~BIT3;
        }
        if(((P6->IN & BIT4) == 0) && ((P6->IE & BIT4) == BIT4))
        {
            P1->OUT ^= BIT0;
            P6->IE &= ~BIT4;
        }
        if(((P6->IN & BIT5) == 0) && ((P6->IE & BIT5) == BIT5))
        {
            P1->OUT ^= BIT0;
            P6->IE &= ~BIT5;
        }
        if(P6->IE == BIT1)
        {
            if(sequence)
            {
                victorySequence();
                sequence = 0;
            }
        }
    }
}


/* Reset Switch */
void PORT1_IRQHandler(void)
{
    P1->IFG = 0;
    /* turn on LEDs */
    P3->OUT |= LED1;/* turn on LED1 */
    P5->OUT |= LED2;/* turn on LED2 */
    P5->OUT |= LED3;/* turn on LED3 */
    P1->OUT |= LED4;/* turn on LED4 */
    P1->OUT |= LED5;/* turn on LED5 */

    /* reenable wires */
    P6->IE |= (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5);

    /* reset timer */
    timerStage = 5;

    /* Randomize again */
    rand = -1;

    /* Allow another victory sequence */
    sequence = 1;

    /* Enable Timer32 */
    TIMER32_1->CONTROL |= TIMER32_CONTROL_ENABLE;
}

/* LED Countdown */
void T32_INT1_IRQHandler(void)
{
  TIMER32_1->INTCLR = 1;/* clear the interrupt flag for Timer32 module 0 */
  if (timerStage == 1)
  {
      P3->OUT &= ~LED1;/* turn off LED1 */
      timerStage--;
  }
  else if (timerStage == 2)
  {
      P5->OUT &= ~LED2;/* turn off LED2 */
      timerStage--;
  }
  else if (timerStage == 3)
  {
      P5->OUT &= ~LED3;/* turn off LED3 */
      timerStage--;
  }
  else if (timerStage == 4)
  {
      P1->OUT &= ~LED4;/* turn off LED4 */
      timerStage--;
  }
  else if (timerStage == 5)
  {
      P1->OUT &= ~LED5;/* turn off LED5 */
      timerStage--;
  }
  else
  {
      defeatSequence();
  }
}
