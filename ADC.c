//*****************************************************************************
//
// uart_echo.c - Example for reading data from and writing data to the UART in
//               an interrupt driven fashion.
//
// Copyright (c) 2011-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.4.178 of the DK-TM4C123G Firmware Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "grlib/grlib.h"
#include "drivers/cfal96x64x16.h"

//new includes
#include "driverlib/adc.h"

void zalupa();

uint8_t programExit = 0;
uint32_t loopCount = 0;
uint32_t xmit = 0;
uint32_t recv = 0;

uint32_t ui32Loop;

bool ledToggle = false;
bool countToggle = false;
bool charToogle = false;
bool analogToggle =false;
bool analogToggle1 = false;
bool analogToggle2 = false;
int32_t breaking_char;
int looptoggle = 0;
int counter =0;
uint32_t pui32ADC0Value[3];
tRectangle sRect;
tContext sContext;
void zalupaa();
void zalupaaa();

int p1;
int p2;
int p3;




char buffer[32];



//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>UART Echo (uart_echo)</h1>
//!
//! This example application utilizes the UART to echo text.  The first UART
//! (connected to the USB debug virtual serial port on the evaluation board)
//! will be configured in 115,200 baud, 8-n-1 mode.  All characters received on
//! the UART are transmitted back to the UART.
//
//*****************************************************************************
void
UARTIntHandler(void)
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
        //
        // Read the next character from the UART and write it back to the UART.
        //
        ROM_UARTCharPutNonBlocking(UART0_BASE,
                                   ROM_UARTCharGetNonBlocking(UART0_BASE));
    }
}
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

void ledPrint() {
      sRect.i16XMin = 0;
      sRect.i16YMin = 0;
      sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
      sRect.i16YMax = GrContextDpyHeightGet(&sContext) - 1;
      GrContextForegroundSet(&sContext, ClrBlack);
      GrRectFill(&sContext, &sRect);
    sRect.i16XMin = 0;
    sRect.i16YMin = 0;
    sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
    sRect.i16YMax = 9;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);

    //
    // Change foreground for white text.
    //
    GrContextForegroundSet(&sContext, ClrWhite);

    //
    // Put the application name in the middle of the banner.
    //
    GrContextFontSet(&sContext, g_psFontFixed6x8);
    GrStringDrawCentered(&sContext, "yeeee boiii", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 4, 0);

    //
    // Initialize the display and write some instructions.
    //
    GrStringDrawCentered(&sContext, "This is a ", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 20, false);
    GrStringDrawCentered(&sContext, "unique", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 30, false);
    GrStringDrawCentered(&sContext, "identifier", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 40, false);
    GrStringDrawCentered(&sContext, "yes", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 50, false);
}

//*****************************************************************************
//
// Send a string to the UART.
//
//*****************************************************************************
void
UARTSend(const uint8_t *pui8Buffer, uint32_t ui32Count)
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
        xmit++;
    }
}

char UARTReceive(uint32_t ui32Base)
{
    int32_t local_char;
    local_char=UARTCharGetNonBlocking(ui32Base);
    recv++;
    
    return local_char;
}

void printMenu() {
    UARTSend((uint8_t *)"Enter your menu selection:\r\n", 30);
    UARTSend((uint8_t *)"M - print this Menu\r\n", 23);
    UARTSend((uint8_t *)"Q - Quit the program\r\n", 24);
    UARTSend((uint8_t *)"L - toggle Loop count display\r\n", 32);
    UARTSend((uint8_t *)"B - toggle Button display\r\n", 28);
    UARTSend((uint8_t *)"U - toggle UART recv/xmit display\r\n", 36);
    UARTSend((uint8_t *)"F - toggle Flood character enable\r\n", 36);
    UARTSend((uint8_t *)"D - toggle flashing LED\r\n", 26);
    UARTSend((uint8_t *)"C - Clear terminal window\r\n", 27);
}

void splashScreen() {
  sRect.i16XMin = 0;
  sRect.i16YMin = 0;
  sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
  sRect.i16YMax = GrContextDpyHeightGet(&sContext) - 1;
  GrContextForegroundSet(&sContext, ClrBlueViolet);
  GrRectFill(&sContext, &sRect);
  GrContextFontSet(&sContext, g_psFontCm12);
  GrContextForegroundSet(&sContext, ClrWhite);
  
  GrStringDrawCentered(&sContext, "This is a ", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 20, false);
    GrStringDrawCentered(&sContext, "Splash", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 30, false);
    GrStringDrawCentered(&sContext, "Screen", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 40, false);
    GrStringDrawCentered(&sContext, "yes", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 50, false);
}

void blackScreen() {
      sRect.i16XMin = 0;
      sRect.i16YMin = 0;
      sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
      sRect.i16YMax = GrContextDpyHeightGet(&sContext) - 1;
      GrContextForegroundSet(&sContext, ClrBlack);
      GrRectFill(&sContext, &sRect);
}

void blackScreen2(int xMin, int xMax, int yMin, int yMax) {
      sRect.i16XMin = xMin;
      sRect.i16YMin = yMin;
      sRect.i16XMax = xMax;
      sRect.i16YMax = yMax;
      GrContextForegroundSet(&sContext, ClrBlack);
      GrRectFill(&sContext, &sRect);
}

void startScreen() {
  sRect.i16XMin = 0;
    sRect.i16YMin = 0;
    sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
    sRect.i16YMax = 9;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);

    //
    // Change foreground for white text.
    //
    GrContextForegroundSet(&sContext, ClrWhite);

    //
    // Put the application name in the middle of the banner.
    //
    GrContextFontSet(&sContext, g_psFontFixed6x8);
    GrStringDrawCentered(&sContext, "yeeee boiii", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 4, 0);
      GrContextForegroundSet(&sContext, ClrWhite);
      GrStringDrawCentered(&sContext, "This is a ", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 20, false);
    GrStringDrawCentered(&sContext, "unique", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 30, false);
    GrStringDrawCentered(&sContext, "identifier", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 40, false);
    GrStringDrawCentered(&sContext, "yes", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 50, false);
}
  /*
void drawBar1
blackScreen();
  ADCProcessorTrigger(ADC0_BASE, 1);

      //
      // Wait for conversion to be completed.
      //
      while(!ADCIntStatus(ADC0_BASE, 1, false))
      {
      }

      //
      // Clear the ADC interrupt flag.
      //
      ADCIntClear(ADC0_BASE, 1);

      //
      // Read ADC Value.
      //
      ADCSequenceDataGet(ADC0_BASE, 1, pui32ADC0Value);
      

      //
      // Display the AIN0 (PE3) digital value on the console.
      //
  int maxWidth = GrContextDpyWidthGet(&sContext);
  sprintf(buffer,"1 =%d\r",pui32ADC0Value[0]);
    GrStringDrawCentered(&sContext, buffer, -1,
                        GrContextDpyWidthGet(&sContext) / 2, 25, 1);
    
  sRect.i16XMin = 0;
    sRect.i16YMin = 15;
    sRect.i16XMax = (uint32_t)(maxWidth * ((float)pui32ADC0Value[0] / 4095));
    sRect.i16YMax = 20;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);
}

void drawBar2() {
  blackScreen();
  ADCProcessorTrigger(ADC0_BASE, 1);

      //
      // Wait for conversion to be completed.
      //
      while(!ADCIntStatus(ADC0_BASE, 1, false))
      {
      }

      //
      // Clear the ADC interrupt flag.
      //
      ADCIntClear(ADC0_BASE, 1);

      //
      // Read ADC Value.
      //
      ADCSequenceDataGet(ADC0_BASE, 1, pui32ADC0Value);
      

      //
      // Display the AIN0 (PE3) digital value on the console.
      //
  int maxWidth = GrContextDpyWidthGet(&sContext);
  sprintf(buffer,"2 =%d\r",pui32ADC0Value[1]);
    GrStringDrawCentered(&sContext, buffer, -1,
                        GrContextDpyWidthGet(&sContext) / 2, 25, 1);
    
  sRect.i16XMin = 0;
    sRect.i16YMin = 15;
    sRect.i16XMax = (uint32_t)(maxWidth * ((float)pui32ADC0Value[0] / 4095));
    sRect.i16YMax = 20;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);
}

void drawBar3() {
  blackScreen();
  ADCProcessorTrigger(ADC0_BASE, 1);

      //
      // Wait for conversion to be completed.
      //
      while(!ADCIntStatus(ADC0_BASE, 1, false))
      {
      }

      //
      // Clear the ADC interrupt flag.
      //
      ADCIntClear(ADC0_BASE, 1);

      //
      // Read ADC Value.
      //
      ADCSequenceDataGet(ADC0_BASE, 1, pui32ADC0Value);
      

      //
      // Display the AIN0 (PE3) digital value on the console.
      //
  int maxWidth = GrContextDpyWidthGet(&sContext);
  sprintf(buffer,"3 =%d\r",pui32ADC0Value[2]);
    GrStringDrawCentered(&sContext, buffer, -1,
                        GrContextDpyWidthGet(&sContext) / 2, 25, 1);
    
  sRect.i16XMin = 0;
    sRect.i16YMin = 15;
    sRect.i16XMax = (uint32_t)(maxWidth * ((float)pui32ADC0Value[0] / 4095));
    sRect.i16YMax = 20;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);
}
*/
void process_menu (int32_t character) {
//  int maxWidth = GrContextDpyWidthGet(&sContext);
  switch(character) {
  case 'S':
    splashScreen();
    SysCtlDelay(10000000);
    blackScreen();
    startScreen();
    break;
  case 'M':
    printMenu();
    break;
  case 'F':
    while(1){
    SysCtlDelay(1000000);
    UARTCharPutNonBlocking(UART0_BASE, character);
    if(UARTCharsAvail(UART0_BASE)){
      int32_t breaking_char;
      breaking_char=UARTReceive(UART0_BASE);
    if(breaking_char =='F') 
      break;
      }
    }
    break;
  case 'Q':
    programExit = 1;
    break;
  case 'L':
    countToggle = !countToggle;
    char boof[12];
    while (countToggle == true){ 
    loopCount++;
      sprintf(boof, "%d", loopCount);
      sRect.i16XMin = 0;
      sRect.i16YMin = 0;
      sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
      sRect.i16YMax = GrContextDpyHeightGet(&sContext) - 1;
      GrContextForegroundSet(&sContext, ClrBlack);
      GrRectFill(&sContext, &sRect);
      GrContextFontSet(&sContext, g_psFontCm12);
      GrContextForegroundSet(&sContext, ClrWhite);
      GrStringDrawCentered(&sContext, boof, -1,
                           GrContextDpyWidthGet(&sContext) / 2, 10, 0);
      
     if(UARTCharsAvail(UART0_BASE)){
      int32_t breaking_char;
      breaking_char=UARTReceive(UART0_BASE);
    if(breaking_char =='L') 
      break;
    else process_menu(breaking_char);
    }
    }
    break;
      
  case 'U':
    charToogle =!charToogle;
    while(charToogle == true) {
      sRect.i16XMin = 0;
      sRect.i16YMin = 0;
      sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
      sRect.i16YMax = GrContextDpyHeightGet(&sContext) - 1;
      GrContextForegroundSet(&sContext, ClrBlack);
      GrRectFill(&sContext, &sRect);
      GrContextFontSet(&sContext, g_psFontCm12);
      sprintf(boof, "%d", xmit);
      GrContextForegroundSet(&sContext, ClrWhite);
      GrStringDrawCentered(&sContext, boof, -1,
                           GrContextDpyWidthGet(&sContext) / 2, 10, 0);
      sprintf(boof, "%d", recv);
      GrStringDrawCentered(&sContext, boof, -1,
                           GrContextDpyWidthGet(&sContext) / 2, 20, 0);
    if(UARTCharsAvail(UART0_BASE)){
      int32_t breaking_char;
      breaking_char=UARTReceive(UART0_BASE);
    if(breaking_char =='U') 
      break;
    else process_menu(breaking_char);
    }
    }
    break;
  case 'B':
    break;
  case 'D':
    ledToggle = !ledToggle;
    while (ledToggle == true) {
    GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_2, GPIO_PIN_2);
    SysCtlDelay(1000000);
    GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_2, 0);
    SysCtlDelay(1000000);
    
    if(UARTCharsAvail(UART0_BASE)){
      int32_t breaking_char;
      breaking_char=UARTReceive(UART0_BASE);
    if(breaking_char =='D') 
      break;
    else process_menu(breaking_char);
    }
    }
    
    break;
    
    
  case 'C':
    UARTSend((uint8_t *)"\033[2J\r", 7);
    break;
  case 'A':
    analogToggle = !analogToggle;
    while(analogToggle == true) {
    zalupa();
    
    if(UARTCharsAvail(UART0_BASE)){
      int32_t breaking_char;
      breaking_char=UARTReceive(UART0_BASE);
  if(breaking_char =='A') 
      break;
    else process_menu(breaking_char);
    }
    }
    
    break;
  case 'a':
    analogToggle1 = !analogToggle1;
    while (analogToggle1 == true) {
      zalupaa();
      if(UARTCharsAvail(UART0_BASE)){
      int32_t breaking_char;
      breaking_char=UARTReceive(UART0_BASE);
  if(breaking_char =='a') 
      break;
    else process_menu(breaking_char);
    }
    }
    
    break;
    case 'w':
    analogToggle2 = !analogToggle2;
    while (analogToggle2 == true) {
      zalupaaa();
      if(UARTCharsAvail(UART0_BASE)){
      int32_t breaking_char;
      breaking_char=UARTReceive(UART0_BASE);
  if(breaking_char =='w') 
      break;
    else process_menu(breaking_char);
    }
    }
    
    break;
      
    
  
}
}
//*****************************************************************************
//
// This example demonstrates how to send a string of data to the UART.
//
//*****************************************************************************
void zalupaaa() {
  ADCProcessorTrigger(ADC0_BASE, 1);

      //
      // Wait for conversion to be completed.
      //
      while(!ADCIntStatus(ADC0_BASE, 1, false))
      {
      }

      //
      // Clear the ADC interrupt flag.
      //
      ADCIntClear(ADC0_BASE, 1);

      //
      // Read ADC Value.
      //
      ADCSequenceDataGet(ADC0_BASE, 1, pui32ADC0Value);
      blackScreen();
  int maxWidth = GrContextDpyWidthGet(&sContext);
    
    
    sRect.i16XMin = 0;
    sRect.i16YMin = 15;
    sRect.i16XMax = (uint32_t)(maxWidth * ((float)pui32ADC0Value[0] / 4095));
    sRect.i16YMax = 20;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);
    
    sRect.i16XMin = 0;
    sRect.i16YMin = 30;
    sRect.i16XMax = (uint32_t)(maxWidth * ((float)pui32ADC0Value[1] / 4095));
    sRect.i16YMax = 35;
    GrContextForegroundSet(&sContext, ClrGreen);
    GrRectFill(&sContext, &sRect);
    
    sRect.i16XMin = 0;
    sRect.i16YMin = 45;
    sRect.i16XMax = (uint32_t)(maxWidth * ((float)pui32ADC0Value[2] / 4095));
    sRect.i16YMax = 50;
    GrContextForegroundSet(&sContext, ClrPurple);
    GrRectFill(&sContext, &sRect);
}

void zalupaa() {
  ADCProcessorTrigger(ADC0_BASE, 1);

      //
      // Wait for conversion to be completed.
      //
      while(!ADCIntStatus(ADC0_BASE, 1, false))
      {
      }

      //
      // Clear the ADC interrupt flag.
      //
      ADCIntClear(ADC0_BASE, 1);

      //
      // Read ADC Value.
      //
      ADCSequenceDataGet(ADC0_BASE, 1, pui32ADC0Value);
      

      //
      // Display the AIN0 (PE3) digital value on the console.
      //
        
      
    sprintf(buffer,"1 =%d\r",pui32ADC0Value[0]);
    GrStringDrawCentered(&sContext, buffer, -1,
                        GrContextDpyWidthGet(&sContext) / 2, 25, 1);
    
    sprintf(buffer,"2 =%d\r",pui32ADC0Value[1]);
    GrStringDrawCentered(&sContext, buffer, -1,
                        GrContextDpyWidthGet(&sContext) / 2, 40, 1);
      
    sprintf(buffer,"3 =%d\r",pui32ADC0Value[2]);
    GrStringDrawCentered(&sContext, buffer, -1,
                        GrContextDpyWidthGet(&sContext) / 2, 55, 1);
    
    blackScreen();
    
    sRect.i16XMin = 0;
    sRect.i16YMin = 0;
    sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
    sRect.i16YMax = 9;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);
    GrContextForegroundSet(&sContext, ClrWhite);
    GrStringDrawCentered(&sContext, "Analog Display", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 4, 0);
}
void zalupa() {
    ADCProcessorTrigger(ADC0_BASE, 1);

      //
      // Wait for conversion to be completed.
      //
      while(!ADCIntStatus(ADC0_BASE, 1, false))
      {
      }

      //
      // Clear the ADC interrupt flag.
      //
      ADCIntClear(ADC0_BASE, 1);

      //
      // Read ADC Value.
      //
      ADCSequenceDataGet(ADC0_BASE, 1, pui32ADC0Value);
      

      //
      // Display the AIN0 (PE3) digital value on the console.
      //
        
      
    sprintf(buffer,"1 =%d\r",pui32ADC0Value[0]);
    GrStringDrawCentered(&sContext, buffer, -1,
                        GrContextDpyWidthGet(&sContext) / 2, 25, 1);
    
    sprintf(buffer,"2 =%d\r",pui32ADC0Value[1]);
    GrStringDrawCentered(&sContext, buffer, -1,
                        GrContextDpyWidthGet(&sContext) / 2, 40, 1);
      
    sprintf(buffer,"3 =%d\r",pui32ADC0Value[2]);
    GrStringDrawCentered(&sContext, buffer, -1,
                        GrContextDpyWidthGet(&sContext) / 2, 55, 1);
    
    blackScreen();
    
    sRect.i16XMin = 0;
    sRect.i16YMin = 0;
    sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
    sRect.i16YMax = 9;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);
    GrContextForegroundSet(&sContext, ClrWhite);
    GrStringDrawCentered(&sContext, "Analog Display", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 4, 0);
    
    int maxWidth = GrContextDpyWidthGet(&sContext);
    
    
    sRect.i16XMin = 0;
    sRect.i16YMin = 15;
    sRect.i16XMax = (uint32_t)(maxWidth * ((float)pui32ADC0Value[0] / 4095));
    sRect.i16YMax = 20;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);
    
    sRect.i16XMin = 0;
    sRect.i16YMin = 30;
    sRect.i16XMax = (uint32_t)(maxWidth * ((float)pui32ADC0Value[1] / 4095));
    sRect.i16YMax = 35;
    GrContextForegroundSet(&sContext, ClrGreen);
    GrRectFill(&sContext, &sRect);
    
    sRect.i16XMin = 0;
    sRect.i16YMin = 45;
    sRect.i16XMax = (uint32_t)(maxWidth * ((float)pui32ADC0Value[2] / 4095));
    sRect.i16YMax = 50;
    GrContextForegroundSet(&sContext, ClrPurple);
    GrRectFill(&sContext, &sRect);
    
   }

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
    //Enable and wait for the port
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
    {
    }
    
    //
    // Initialize and wait for the ADC peripheral
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0))
    {
    }
    
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_5);
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_6);
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_7);
    
    
    

    
   
   
       
    

    
    
    
    
    
    // Initialize the display driver.
    //
    CFAL96x64x16Init();

    //
    // Initialize the graphics context.
    //
    GrContextInit(&sContext, &g_sCFAL96x64x16);
    
    splashScreen();
    SysCtlDelay(10000000);
    blackScreen();
    
    /*
    //
    // Fill the top part of the screen with blue to create the banner.
    //
    sRect.i16XMin = 0;
    sRect.i16YMin = 0;
    sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
    sRect.i16YMax = 9;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);

    //
    // Change foreground for white text.
    //
    GrContextForegroundSet(&sContext, ClrWhite);

    //
    // Put the application name in the middle of the banner.
    //
    GrContextFontSet(&sContext, g_psFontFixed6x8);
    GrStringDrawCentered(&sContext, "yeeee boiii", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 4, 0);

    //
    // Initialize the display and write some instructions.
    //
    
    GrStringDrawCentered(&sContext, "This is a ", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 20, false);
    GrStringDrawCentered(&sContext, "unique", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 30, false);
    GrStringDrawCentered(&sContext, "identifier", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 40, false);
    GrStringDrawCentered(&sContext, "yes", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 50, false);
*/
    startScreen();
    //
    // Enable the peripherals used by this example.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Enable processor interrupts.
    //
    //ROM_IntMasterEnable();

    //
    // Set GPIO A0 and A1 as UART pins.
    //
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Configure the UART for 115,200, 8-N-1 operation.
    //
    ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

    //
    // Enable the UART interrupt.
    //
    //ROM_IntEnable(INT_UART0);
    //ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);

    //
    // Prompt for text to be entered.
    //
    UARTSend((uint8_t *)"Enter your menu selection:\r\n", 30);
    UARTSend((uint8_t *)"M - print this Menu\r\n", 23);
    UARTSend((uint8_t *)"Q - Quit the program\r\n", 24);
    UARTSend((uint8_t *)"L - toggle Loop count display\r\n", 32);
    UARTSend((uint8_t *)"B - toggle Button display\r\n", 28);
    UARTSend((uint8_t *)"U - toggle UART recv/xmit display\r\n", 36);
    UARTSend((uint8_t *)"F - toggle Flood character enable\r\n", 36);
    UARTSend((uint8_t *)"D - toggle flashing LED\r\n", 26);
    UARTSend((uint8_t *)"C - Clear terminal window\r\n", 27);

    //
    // Loop forever echoing data through the UART.
    //
    
    
    //SEQUENCE ADC
    
    
   ADCSequenceDisable(ADC0_BASE,1);
   //pin 7
   ADCSequenceConfigure(ADC0_BASE,1,
                        ADC_TRIGGER_PROCESSOR,1);
   ADCSequenceStepConfigure(ADC0_BASE,1,0,ADC_CTL_CH4);
   ADCSequenceStepConfigure(ADC0_BASE,1,1,ADC_CTL_CH5);
   ADCSequenceStepConfigure(ADC0_BASE,1,2,ADC_CTL_CH6 | ADC_CTL_IE |
                             ADC_CTL_END);
   
   ADCSequenceEnable(ADC0_BASE,1);
   
   
    
    
    
    
    while(1) {
      
      loopCount++;
      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
      while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG))
    {
    }
    GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_2);
      while (UARTCharsAvail(UART0_BASE)) {
        if (programExit == 1)
          break;
        int32_t local_char;
        local_char=UARTReceive(UART0_BASE);
        process_menu(local_char);
      }
    }
}
