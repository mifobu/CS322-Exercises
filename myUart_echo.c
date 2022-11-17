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

uint8_t programExit = 0;
uint32_t loopCount = 0;
uint32_t xmit = 0;
 uint32_t recv = 0;

 uint32_t ui32Loop;

bool ledToggle = false;
bool countToggle = false;
bool charToogle = false;
int32_t breaking_char;

tRectangle sRect;
tContext sContext;

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

void process_menu (int32_t character) {
  
  switch(character) {
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
  
}
}
//*****************************************************************************
//
// This example demonstrates how to send a string of data to the UART.
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
    // Initialize the graphics context.
    //
    GrContextInit(&sContext, &g_sCFAL96x64x16);

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
