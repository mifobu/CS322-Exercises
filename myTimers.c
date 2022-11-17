//*****************************************************************************
/*
Starting project - myTimers.c
Arseniy Gorbanev

Lab 7 Excercise 
•	Ability to switch between two drive patterns through user input
•	Ability to reverse directions and stop the motor through user input
•	Ability to toggle follower mode where the motor follows the ADC potentiometer input
•	Display a personalized splash screen on startup as well as through user input

Completed on 19/11/2021
*/
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "grlib/grlib.h"
#include "drivers/cfal96x64x16.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"


tRectangle sRect;
tContext sContext;
int ui32Loop;
char buffer0[32];

bool ADCtoggle = true;
bool modeToggle = true; //direction toggle
bool followerToggle = false; //mode toggle
bool offToggle = true; //On/off toggle
bool modeSelect = false; //Drive pattern selector toggle
char UARTReceive(uint32_t ui32Base);
void process_menu(int character);
uint32_t pui32ADC0Value[1];
bool ADCflag = false;
uint32_t ADCvalue;
uint32_t currentStep = 0;
uint32_t div = 0;


uint32_t p_iter = 0;

//driveshit
const uint8_t full_step_array[4] = {0x0C, 0x06, 0x03, 0x09};
const uint8_t full_step_array_reverse[4] = {0x09, 0x03, 0x06, 0x0C};
const uint8_t wave_drive_array[4] = {0x08, 0x04, 0x02, 0x01};
const uint8_t wave_drive_array_reverse[4] = {0x01, 0x02, 0x04, 0x08};


uint32_t freq = 0;
#define MOTOR_OFF (0x00)
#define STEPS_PER_REV 200
#define INIT_RPM (60)
#define RPM_MIN (1)
#define RPM_MAX (250)
int RPM = 60;

void blinky();

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Timer (timers)</h1>
//!
//! This example application demonstrates the use of the timers to generate
//! periodic interrupts.  One timer is set up to interrupt once per second and
//! the other to interrupt twice per second; each interrupt handler will toggle
//! its own indicator on the display.
//
//*****************************************************************************

//*****************************************************************************
//
// Flags that contain the current value of the interrupt indicator as displayed
// on the CSTN display.
//
//*****************************************************************************
uint32_t g_ui32Flags;

//*****************************************************************************
//
// Graphics context used to show text on the CSTN display.
//
//*****************************************************************************
tContext g_sContext;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//*****************************************************************************
//
// The interrupt handler for the first timer interrupt.
//

void
Timer0IntHandler(void)
{
    //
    // Clear the timer interrupt.
    //
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    //
    // Toggle the flag for the first timer.
    //
    
    //HWREGBITW(&g_ui32Flags, 0) ^= 1;
    
    //ADC 
   ADCProcessorTrigger(ADC0_BASE, 1);
        ADCSequenceDataGet(ADC0_BASE, 1, pui32ADC0Value);
        ADCvalue = pui32ADC0Value[0];
        sprintf(buffer0, "%d    ", ADCvalue);
        IntMasterDisable();
        GrStringDraw(&g_sContext, buffer0, -1, 40,
                     46, 1);
        IntMasterEnable();
        int ADCStep = (ADCvalue*200)/4096;
    


    if (offToggle) {
    GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                 MOTOR_OFF);
    } else if (followerToggle) {
        
        if (ADCStep > currentStep) {
          currentStep++;
          GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                   full_step_array[p_iter++%4]);
        } else if (ADCStep < currentStep) {
          currentStep--;
          GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                   full_step_array_reverse[p_iter++%4]);
        } else {
          GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                 MOTOR_OFF);
        }
          
    } else if (modeSelect) {
        currentStep = modeToggle ? currentStep+1 : currentStep-1;
        
        currentStep=currentStep%200;
        if (modeToggle) {
            GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                   full_step_array[p_iter++%4]);
        } else {
            GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                   full_step_array_reverse[p_iter++%4]);
        }
    } else if (!modeSelect) {
        currentStep = modeToggle ? currentStep+1 : currentStep-1;
        
        currentStep=currentStep%200;
        if (modeToggle) {
            GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                   wave_drive_array[p_iter++%4]);            
        } else {
            GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                   wave_drive_array_reverse[p_iter++%4]);
        }
    }
}

//Black Screen
void blackScreen() {
      sRect.i16XMin = 0;
      sRect.i16YMin = 0;
      sRect.i16XMax = GrContextDpyWidthGet(&g_sContext) - 1;
      sRect.i16YMax = GrContextDpyHeightGet(&g_sContext) - 1;
      GrContextForegroundSet(&g_sContext, ClrBlack);
      GrRectFill(&g_sContext, &sRect);
}

//LED initialization
void initBlinky() {
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG))
    {
    }
  GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_2);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
    {
    }
}

//ADC initialization
void initADC() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0))
    {
    }
    
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_5);
    
    ADCSequenceDisable(ADC0_BASE,1);
    ADCSequenceConfigure(ADC0_BASE,1,
                        ADC_TRIGGER_PROCESSOR,1);
    ADCSequenceStepConfigure(ADC0_BASE,1,0,ADC_CTL_CH6 | ADC_CTL_IE |
                             ADC_CTL_END);
   
    ADCSequenceEnable(ADC0_BASE,1);
}

//UART initialization
void uartInit() {
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
  ROM_IntEnable(INT_UART0);
  ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
}

//UART Init Handler
void UARTIntHandler(void)
{
    uint32_t ui32Status;

    //
    // Get the interrrupt status.
    //
    ui32Status = ROM_UARTIntStatus(UART0_BASE, true);

    //
    // Clear the asserted interrupts.
    //
    ROM_UARTIntClear(UART0_BASE, ui32Status);

    //
    // Loop while there are characters in the receive FIFO.
    //
    while(ROM_UARTCharsAvail(UART0_BASE))
    {
        int32_t local_char=UARTReceive(UART0_BASE);
        process_menu(local_char);
        //
        // Read the next character from the UART and write it back to the UART.
        //
        //ROM_UARTCharPutNonBlocking(UART0_BASE,
                                   //ROM_UARTCharGetNonBlocking(UART0_BASE));
    }
}
  


//Splash Screen 
void splash() {
  sRect.i16XMin = 0;
  sRect.i16YMin = 0;
  sRect.i16XMax = GrContextDpyWidthGet(&g_sContext) - 1;
  sRect.i16YMax = GrContextDpyHeightGet(&g_sContext) - 1;
  GrContextForegroundSet(&g_sContext, ClrBlueViolet);
  GrRectFill(&g_sContext, &sRect);
  GrContextFontSet(&g_sContext, g_psFontCm12);
  GrContextForegroundSet(&g_sContext, ClrWhite);
  
  GrStringDrawCentered(&g_sContext, "Splash ", -1,
                         GrContextDpyWidthGet(&g_sContext) / 2, 20, false);
  GrStringDrawCentered(&g_sContext, "Screen", -1,
                       GrContextDpyWidthGet(&g_sContext) / 2, 30, false);
  GrStringDrawCentered(&g_sContext, "Arseniy", -1,
                       GrContextDpyWidthGet(&g_sContext) / 2, 40, false);
  GrStringDrawCentered(&g_sContext, "Gorbanev", -1,
                       GrContextDpyWidthGet(&g_sContext) / 2, 50, false);
  SysCtlDelay(2000000);
  blackScreen();
  sRect.i16XMin = 0;
    sRect.i16YMin = 0;
    sRect.i16XMax = GrContextDpyWidthGet(&g_sContext) - 1;
    sRect.i16YMax = 9;
    GrContextForegroundSet(&g_sContext, ClrDarkBlue);
    GrRectFill(&g_sContext, &sRect);

    //
    // Change foreground for white text.
    //
    GrContextForegroundSet(&g_sContext, ClrWhite);

    //
    // Put the application name in the middle of the banner.
    //
    GrContextFontSet(&g_sContext, g_psFontFixed6x8);
    GrStringDrawCentered(&g_sContext, "Motors", -1,
                         GrContextDpyWidthGet(&g_sContext) / 2, 4, 0);

    //
    // Initialize timer status display.
    //
    GrContextFontSet(&g_sContext, g_psFontFixed6x8);
    GrStringDraw(&g_sContext, "RPM:", -1, 1, 26, 0);
    GrStringDraw(&g_sContext, "ADC:", -1, 1, 46, 0);
}

void blinky() {
        //
        // Turn on the LED.
        //
        GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_2, GPIO_PIN_2);

        //
        // Delay for a bit.
        //
        for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++)
        {
        }

        //
        // Turn off the LED.
        //
        GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_2, 0);

        //
        // Delay for a bit.
        //
        for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++)
        {
        }
}

//UARTSend

void UARTSend(const uint8_t *pui8Buffer, uint32_t ui32Count)
{
    //
    // Loop while there are more characters to send.
    //
    while(ui32Count--)
    {
        //
        // Write the next character to the UART.
        //
        ROM_UARTCharPut(UART0_BASE, *pui8Buffer++);
    }
}

char UARTReceive(uint32_t ui32Base)
{
    int32_t local_char;
    local_char=UARTCharGetNonBlocking(ui32Base);
    return local_char;
}

void printMenu() {
    UARTSend((uint8_t *)"Enter your menu selection:\r\n", 30);
    UARTSend((uint8_t *)"M/m - print this Menu\r\n", 23);
    UARTSend((uint8_t *)"C/c - Clear terminal window\r\n", 29);
    UARTSend((uint8_t *)"D/d - Constant velocity mode\r\n", 30);
    UARTSend((uint8_t *)"F/f - Follower velocity mode\r\n", 30);
    UARTSend((uint8_t *)"S/s - Stop\r\n", 12);
    UARTSend((uint8_t *)"P/p - print splash screen\r\n", 27);
}

void clear() {
  UARTSend((uint8_t *)"\033[2J\r", 7);
}

void process_menu(int character) {
  switch(character) {
      case'Z':
      case 'z':
        followerToggle=!followerToggle;
        break;
      case '+':
        //Increment the RPM if its not at minimum, set new frequency for the ISR
        //and display the new value on the display
        if (RPM <= RPM_MAX) {
            RPM++;
            div=(RPM*200)/60;
            
            TimerLoadSet(TIMER0_BASE, TIMER_A, (SysCtlClockGet()/div));
            IntMasterDisable();
            sprintf(buffer0, "%d    ", RPM);
            GrStringDraw(&g_sContext, buffer0, -1, 40,
                     26, 1);
            IntMasterEnable();
          }
        break;
      case '-':
        //Decrement the RPM if its not at minimum, set new frequency for the ISR
        //and display the new value on the display
        if (RPM >= RPM_MIN) {
            RPM--;
            div=(RPM*200)/60;
            TimerLoadSet(TIMER0_BASE, TIMER_A, (SysCtlClockGet()/div));
            IntMasterDisable();
            sprintf(buffer0, "%d    ", RPM);
            GrStringDraw(&g_sContext, buffer0, -1, 40,
                     26, 1);
            IntMasterEnable();
          }
        break;
      case 'D':
      case 'd':
        modeToggle = !modeToggle;
        break;
      case 'R':
      case 'r':
        modeSelect=!modeSelect;
        break;
      case 'S':
      case 's':
        offToggle=!offToggle;
        break;
      case 'M':
      case 'm':
        printMenu();
        break;
      case 'C':
      case 'c':
        clear();
        break;
      case 'P':
      case 'p':
        splash();
        break;
      }
}

//Motor interface GPIO initialization
void pinInit() {
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL))
  {
  }
  GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_0);
  GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_1);
  GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_2);
  GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_3);  
}
//*****************************************************************************
//
// This example application demonstrates the use of the timers to generate
// periodic interrupts.
//
//*****************************************************************************
int
main(void)
{

    //
    // Enable lazy stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    ROM_FPULazyStackingEnable();

    //
    // Set the clocking to run directly from the crystal.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);

    //
    // Initialize the display driver.
    //
    CFAL96x64x16Init();

    //
    // Initialize the graphics context and find the middle X coordinate.
    //
    GrContextInit(&g_sContext, &g_sCFAL96x64x16);
    
    //Calling functionality 
    splash();
    pinInit();
    initBlinky();
    uartInit();
    initADC();
    printMenu();
    
    //
    // Enable the peripherals used by this example.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    //
    // Enable processor interrupts.
    //
    ROM_IntMasterEnable();

    //
    // Configure the two 32-bit periodic timers.
    //
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()/200);

    //
    // Setup the interrupts for the timer timeouts.
    //
    ROM_IntEnable(INT_TIMER0A);
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    //
    // Enable the timers.
    //
    ROM_TimerEnable(TIMER0_BASE, TIMER_A);

    //
    // Loop forever while the timers run.
    //
    while(1)
    {
      
        blinky();
      
    }
}
