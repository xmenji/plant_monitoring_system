//*****************************************************************************
// Copyright (c) 2014 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions
//   are met:
// 
//   Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the  
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// This file was automatically generated by the Tiva C Series PinMux Utility
// Version: 1.0.4
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "system.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"

#define RED_MASK 0x02
#define BLUE_MASK 0x04
#define GREEN_MASK 0x08
//*****************************************************************************
//Variables

//keep track of which menu screen selection the program is on
volatile unsigned long menuNumber = 0; 

uint32_t ui32ADC0Value[4]; //ADC0

//Soil Moisture values (CH0)
uint32_t ui32ADC0ValueCH0[8]; //ADC0 for SS0
volatile uint32_t ui32SoilAvg;
float soilMoistureFloat;
uint32_t soilMoisture;

//Temperature values (CH1)
float Tc = 0.01f; //Temp. coefficient [V/degreeCelsius]
float Voc = 0.5f; //Sensor output V at 0degreeC
float Vout; //Sensor output V
float celsius;
float fahrenheit;
char degreeSymbol = 248;
uint32_t ui32ADC0ValueCH1[4]; //ADC0
volatile uint32_t ui32TempAvg;
volatile uint32_t ui32TempValueC;
volatile uint32_t ui32TempValueF;

//Sunlight values (CH2)
uint32_t ui32ADC0ValueCH2[4]; //ADC0
volatile uint32_t ui32SunlightAvg;


//UART messages
char soilmoisture_str[] = "Soil Moisture:";
char temp_str[] = "Temperature:";
char sunlight_str[] = "Amount of sunlight:";
char default_menu_str[] = "Press either button to make a selection\n\r";


void
PortFunctionInit(void)
{
    //
    // Enable Peripheral Clocks 
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    //
    // Enable pin PE2 for ADC AIN1
    //
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);

    //
    // Enable pin PE1 for ADC AIN2
    //
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1);

    //
    // Enable pin PE3 for ADC AIN0
    //
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    //
    // Enable pin PF4 for GPIOInput
    //
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);

    //
    // Enable pin PF2 for GPIOOutput
    //
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);

    //
    // Enable pin PF0 for GPIOInput
    //

    //
    //First open the lock and select the bits we want to modify in the GPIO commit register.
    //
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = 0x1;

    //
    //Now modify the configuration of the pins that we unlocked.
    //
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);

    //
    // Enable pin PF3 for GPIOOutput
    //
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);

    //
    // Enable pin PF1 for GPIOOutput
    //
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
		
		
		// Enable PF4, and PF0 for digital function.
    GPIO_PORTF_DEN_R |= 0x11;
	
		//Enable pull-up on PF4 and PF0
		GPIO_PORTF_PUR_R |= 0x11; 
}




void
GPIOF_Interrupt_Init(void)
{
  IntEnable(INT_GPIOF);  							// enable interrupt 30 in NVIC (GPIOF)
	IntPrioritySet(INT_GPIOF, 0x00); 		// configure GPIOF interrupt priority as 0
	GPIO_PORTF_IM_R |= 0x11;   		// arm interrupt on PF0 and PF4
	GPIO_PORTF_IS_R &= ~0x11;     // PF0 and PF4 are edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;   	// PF0 and PF4 not both edges trigger 
  GPIO_PORTF_IEV_R &= ~0x11;  	// PF0 and PF4 falling edge event
}



void
uart_Init(void) {


    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);


    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}





//ADC0 initializaiton
void ADC0_Init(void){

		SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);	//activate the clock of ADC0
		SysCtlDelay(2);	//insert a few cycles after enabling the peripheral to allow the clock to be fully activated.
		
		
		ADCSequenceDisable(ADC0_BASE, 1); //disable ADC0 before the configuration is complete
		ADCSequenceDisable(ADC0_BASE, 2); //disable ADC0 before the configuration is complete
	
		
		ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0); // will use ADC0, SS1, processor-trigger, priority 1
		ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 1); // will use ADC0, SS2, processor-trigger, priority 2
		
	
		//Sample from CH1 (Temperature sensor)
		ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH1); //ADC0 SS1 Step 0, sample from temperature sensor
		ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH1); //ADC0 SS1 Step 1, sample from temperature sensor
		ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH1); //ADC0 SS1 Step 2, sample from temperature sensor
		////ADC0 SS1 Step 0, sample from internal temperature sensor, completion of this step will set RIS, last sample of the sequence
		ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_CH1|ADC_CTL_IE|ADC_CTL_END); 
		
		
		//Sample from CH2 (Sunlight sensor)
		ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH2); //ADC0 SS1 Step 0, sample from sunlight
		ADCSequenceStepConfigure(ADC0_BASE, 2, 1, ADC_CTL_CH2); //ADC0 SS1 Step 1, sample from sunlight
		ADCSequenceStepConfigure(ADC0_BASE, 2, 2, ADC_CTL_CH2); //ADC0 SS1 Step 2, sample from sunlight
		////ADC0 SS1 Step 0, sample from internal temperature sensor, completion of this step will set RIS, last sample of the sequence
		ADCSequenceStepConfigure(ADC0_BASE,2,3,ADC_CTL_CH2|ADC_CTL_IE|ADC_CTL_END); 


		IntPrioritySet(INT_ADC0SS1, 0x02);  	 // configure ADC0 SS1 interrupt priority as 0
		IntEnable(INT_ADC0SS1);    				// enable interrupt 31 in NVIC (ADC0 SS1)
		ADCIntEnableEx(ADC0_BASE, ADC_INT_SS1);      // arm interrupt of ADC0 SS1
		
		IntPrioritySet(INT_ADC0SS2, 0x02);  	 // configure ADC0 SS1 interrupt priority as 0
		IntEnable(INT_ADC0SS2);    				// enable interrupt 32 in NVIC (ADC0 SS2)
		ADCIntEnableEx(ADC0_BASE, ADC_INT_SS2);      // arm interrupt of ADC0 SS2
	
		ADCSequenceEnable(ADC0_BASE, 1); //enable ADC0
		ADCSequenceEnable(ADC0_BASE, 2); //enable ADC0
}



//ADC0 interrupt handler [CH1:Temp] 
void ADC0_CH1Handler(void){
		ADCIntClear(ADC0_BASE, 1);
		ADCProcessorTrigger(ADC0_BASE, 1);
	
		ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0ValueCH1);	
	
		//Get avg of sample data
		ui32TempAvg = (ui32ADC0ValueCH1[0] + ui32ADC0ValueCH1[1] + ui32ADC0ValueCH1[2] + ui32ADC0ValueCH1[3]) / 4;
	
		Vout = (2.5f / 4096.0f) * (float)ui32TempAvg; //calculate sensor output voltage
		
		ui32TempValueC = (int)((Vout - Voc) / Tc); //calculate ambient temp [C]
		//fahrenheit = (((float) ui32TempValueC )* 1.8f) + 32.0f;
		//ui32TempValueF = (ui32TempValueC * 2) + 32; //calculate ambient temp [F]
				
}
//ADC0 interrupt handler [CH2:Light]
void ADC0_CH2Handler(void){
		ADCIntClear(ADC0_BASE, 2);
		ADCProcessorTrigger(ADC0_BASE, 2);
	
		ADCSequenceDataGet(ADC0_BASE, 2, ui32ADC0ValueCH2);	
	
		ui32SunlightAvg = (ui32ADC0ValueCH2[0] + ui32ADC0ValueCH2[1] + ui32ADC0ValueCH2[2] + ui32ADC0ValueCH2[3]) / 4;
}



//ADC1 initializaiton
void ADC1_Init(void)
{
	
		//SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ); // configure the system clock to be 40MHz
		SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);	//activate the clock of ADC0
		SysCtlDelay(2);	//insert a few cycles after enabling the peripheral to allow the clock to be fully activated.

		ADCSequenceDisable(ADC1_BASE, 0); //disable ADC0 before the configuration is complete
		ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_PROCESSOR, 0); // will use ADC1, SS0, processor-trigger, priority 0
		//Sample from CH0 (Soil Moisture sensor)
		ADCSequenceStepConfigure(ADC1_BASE, 0, 0, ADC_CTL_CH0); //ADC0 SS1 Step 0, sample from internal temperature sensor
		ADCSequenceStepConfigure(ADC1_BASE, 0, 1, ADC_CTL_CH0); //ADC0 SS1 Step 1, sample from internal temperature sensor
		ADCSequenceStepConfigure(ADC1_BASE, 0, 2, ADC_CTL_CH0); //ADC0 SS1 Step 2, sample from internal temperature sensor
		ADCSequenceStepConfigure(ADC1_BASE, 0, 3, ADC_CTL_CH0); //ADC0 SS1 Step 1, sample from internal temperature sensor
		ADCSequenceStepConfigure(ADC1_BASE, 0, 4, ADC_CTL_CH0); //ADC0 SS1 Step 2, sample from internal temperature sensor
		ADCSequenceStepConfigure(ADC1_BASE, 0, 5, ADC_CTL_CH0); //ADC0 SS1 Step 1, sample from internal temperature sensor
		ADCSequenceStepConfigure(ADC1_BASE, 0, 6, ADC_CTL_CH0); //ADC0 SS1 Step 2, sample from internal temperature sensor
		//ADC0 SS1 Step 0, sample from internal temperature sensor, completion of this step will set RIS, last sample of the sequence
		ADCSequenceStepConfigure(ADC1_BASE,0,7,ADC_CTL_CH0|ADC_CTL_IE|ADC_CTL_END); 
	
		IntPrioritySet(INT_ADC1SS0, 0x00);  	 // configure ADC1 SS0 interrupt priority as 0
		IntEnable(INT_ADC1SS0);    				// enable interrupt 64 in NVIC (ADC1 SS0)
		ADCIntEnableEx(ADC1_BASE, ADC_INT_SS0);      // arm interrupt of ADC1 SS0
	
		ADCSequenceEnable(ADC1_BASE, 0); //enable ADC1
}

//ADC1 interrupt handler [CH0:Soil]
void ADC1_CH0Handler(void){
		ADCIntClear(ADC1_BASE, 0);
		ADCProcessorTrigger(ADC1_BASE, 0);
	
		ADCSequenceDataGet(ADC1_BASE, 0, ui32ADC0ValueCH0);
	
		ui32SoilAvg = (ui32ADC0ValueCH0[0] + ui32ADC0ValueCH0[1] + ui32ADC0ValueCH0[2] + ui32ADC0ValueCH0[3] + ui32ADC0ValueCH0[4] + ui32ADC0ValueCH0[5] + ui32ADC0ValueCH0[6] + ui32ADC0ValueCH0[7]+ 2)/8;
	
		
}





//General method to print UART messages to Tera Term
void printmessage(char str[]){
	int index = 0;
	
	while(str[index] != '\0'){
		UARTCharPut(UART0_BASE, str[index]); 
		index++;
	}
}


void getSoil(){
		if(ui32SoilAvg <= 4095 && ui32SoilAvg > 3071){
			printmessage("The soil is very dry.\n\r");
		}
		else if(ui32SoilAvg <= 3071 && ui32SoilAvg > 2048){
			printmessage("The soil is somewhat dry.\n\r");
		}
		else if(ui32SoilAvg <= 2048 && ui32SoilAvg > 1024){
			printmessage("The soil is somwhat wet.\n\r");
		}
		else if(ui32SoilAvg <= 1024 && ui32SoilAvg > 0){
			printmessage("The soil is very wet.\n\r");
		} 
}

void getSunlight(){
		if(ui32SunlightAvg <= 4095 && ui32SunlightAvg > 3071){
			printmessage("Full Sun.\n\r");
		}
		else if(ui32SunlightAvg <= 3071 && ui32SunlightAvg > 2048){
			printmessage("Mostly Sunny.\n\r");
		}
		else if(ui32SunlightAvg <= 2048 && ui32SunlightAvg > 1024){
			printmessage("Somewhat Sunny.\n\r");
		}
		else if(ui32SunlightAvg <= 1024 && ui32SunlightAvg > 0){
			printmessage("Minimal Sunlight.\n\r");
		} 
}


void GPIOPortF_Handler(void){
	//Disable Interrupt
	IntDisable(INT_GPIOF);
	
	//Delay for a bit
	SysCtlDelay(600000); 
	
	//SW1 is pressed
	if(GPIO_PORTF_RIS_R&0x10)
	{
		// acknowledge flag for PF4
		GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4); 
		//counter decremented by 1
		menuNumber--; //Pressing SW1 makes the option go from right to left.
			
	}
	
	//SW2 is pressed
  else if(GPIO_PORTF_RIS_R&0x01)
	{
		// acknowledge flag for PF0
		GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0);
		//counter incremented by 1
		menuNumber++; //Pressing SW2 makes the option go from left to right.
		
	}
	
	char data[10]; //data acquired from sensors is placed into this cstring for display
	menuNumber &= 3; //rotary counter to implement menu
	
		//implement menu
	switch(menuNumber){
		case 1: //Get soil moisture data
			//turn on green LED
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GREEN_MASK);
			//turn off red & blue LED
			GPIOPinWrite(GPIO_PORTF_BASE, 0x06, 0x00);
		
			getSoil();
			
			
			break;
		
		case 2: //Get temperature data
			//turn on blue LED
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, BLUE_MASK);
			//turn off red & green LEDs
			GPIOPinWrite(GPIO_PORTF_BASE, 0x0A, 0x00);
		
			//calculate degrees in fahrenheit
			//fahrenheit = (((float) ui32TempValueC )* 1.8f) + 32.0f;
			//ui32TempValueF = (int) fahrenheit;
		
			//Print temperature data to screen
			printmessage(temp_str);
		
			//sprintf(data, " %d%cC, %d%cF\n\r", ui32TempValueC, degreeSymbol, ui32TempValueF, degreeSymbol);
			sprintf(data, " %d%cC\n\r", ui32TempValueC, degreeSymbol);
			printmessage(data);
		
			break;
		
		case 3: //Get sunlight exposure data
			//turn on red LED
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, RED_MASK);
			//turn off blue & green LEDs
			GPIOPinWrite(GPIO_PORTF_BASE, 0x0C, 0x00);	
		
			//Print sunlight data to screen
			//printmessage(sunlight_str);
		
			//sprintf(data, " %d\n\r", ui32SunlightAvg);
			//printmessage(data);
			getSunlight();
			break;
		
		default://Default menu
			//turn off all LEDs
			GPIOPinWrite(GPIO_PORTF_BASE, 0x0E, 0x00);
			
			//implement either UART or LCD text here
			printmessage(default_menu_str);
			printmessage("----------------------------------\n\r");
	};
	
	
	//Enable interrupt
	IntEnable(INT_GPIOF); 

}


int main(void){
	
		// configure the system clock to be 40MHz
		SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ); 
	
		//initialize the GPIO ports	
		PortFunctionInit();
	
		//initialize UART0
		uart_Init();
		
		//configure the GPIOF interrupt
		GPIOF_Interrupt_Init();
	
		//initialize ADC0 
		ADC0_Init();
		//initialize ADC1
		ADC1_Init();
		IntMasterEnable();       		// globally enable interrupt
	
		ADCProcessorTrigger(ADC1_BASE, 0);
		ADCProcessorTrigger(ADC0_BASE, 1);
		ADCProcessorTrigger(ADC0_BASE, 2);
	
		//Default menu screen message
		///printmessage(default_menu_str);

	while(1){
	
	}

}