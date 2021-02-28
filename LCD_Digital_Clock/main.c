/*
 * RTC.c
 *
 * Created: 05-01-2021 19:38:45
 * Author : Keyur S. Mandale
 */ 

#define NoofButtons 2		// Macro value set to 2 as two buttons are required to set the clock values
#include <avr/io.h>			
#include <util/twi.h>
#include <stdlib.h>
#include "twi_header.h"		// Two Wire Interface header file
#include "LCDLibrary.h"		// LCD Header file
#include "ButtonLib.h"		// Button Header File for software de-bouncing

uint8_t units_digit = 0;
uint8_t tens_digit  = 0;
uint8_t hour    = 0;
uint8_t seconds = 0;
uint8_t minutes = 0;
uint8_t am_pm   = 0;

uint8_t Update_Value(uint8_t Time_Parameter,uint8_t x, uint8_t y);  // Function to update time 

char LCD_Secs[1];
uint8_t i = 0;

int main(void)
{
    InitializeLCD();			// Initialize LCD in 4 Bit mode
	twi_init();					// Initialize TWI
	DDRD &= ~(1 << PIND6 | 1 << PIND7);   // Configure PIND6 & PIND7 as input pins for buttons
	PORTD |= (1 << PIND6 | 1 << PIND7);	  // Pull PIND6 & PIND7 high 
	
	uint8_t temp_sec = 0;
	uint8_t temp_hr  = 0;
	uint8_t temp_min = 0;
	
	Send_a_String("Time:",0,0); // Initialize the LCD screen with message
	
    while (1) 
    {   

	    units_digit = 0;
	    tens_digit  = 0;
		
		// Read Hours 			
	    temp_hr = twi_ReadRegister(0b00000010);  // Read hours from address of hours register in DS1307
		am_pm   = temp_hr & 0b00100000;			 // Extract Bit 5 from the hours value which is the AM/PM bit. i.e. 1 = PM / 0 = AM
		units_digit = temp_hr & 0b00001111;      // Extract lower nibble of hours register which is the units digit of hours value
		tens_digit  = temp_hr & 0b00010000;		 // Extract upper nibble of hours register which is the tens digit of hours value
		tens_digit = (tens_digit >> 4);			 // Right shift tens_digit by 4 to get correct magnitude of tens digit of hours value
		
		itoa(tens_digit,LCD_Secs,10);            // Convert and send tens digit of hours value
		Send_a_String(LCD_Secs,0,5);			
		itoa(units_digit,LCD_Secs,10);			 // Convert and send units digit of hours value
		Send_a_String(LCD_Secs,0,6);			 
		SendData(0x3A);						     // Send colon(:)
		
		hour = (tens_digit * 10) + units_digit;  // Storing hour value in decimal for updating process	
		
		// Read Minutes
	    temp_min = twi_ReadRegister(0b00000001); // Read minutes from address of minutes register in DS1307
	    units_digit = 0;
	    tens_digit  = 0;
	    units_digit = temp_min & 0b00001111;
	    tens_digit  = temp_min & 0b01110000;
	    tens_digit = (tens_digit >> 4);
		
	    itoa(tens_digit,LCD_Secs,10);
	    Send_a_String(LCD_Secs,0,8);	    
	    itoa(units_digit,LCD_Secs,10);
	    Send_a_String(LCD_Secs,0,9);
	    SendData(0x3A);
		
        minutes = (tens_digit * 10) + units_digit;
	
		//Read Seconds		
		temp_sec = twi_ReadRegister(0b00000000);	// Read seconds from address of seconds register in DS1307
	    units_digit = 0;
		tens_digit  = 0;	
		units_digit = temp_sec & 0b00001111;
		tens_digit = temp_sec & 0b01110000;
		tens_digit = (tens_digit >> 4);
		
		itoa(tens_digit,LCD_Secs,10);
        Send_a_String(LCD_Secs,0,0x0B);
		
		itoa(units_digit,LCD_Secs,10);
		Send_a_String(LCD_Secs,0,0x0C);
		seconds = (tens_digit * 10) + units_digit;
		
		// Display AM/PM depending on value of am_pm
		if (am_pm)
		{
			Send_a_String(" PM",0,0x0D);    // if am_pm is high i.e. Bit 5 of hours register is 1. Display PM
		}
		else
		{
			Send_a_String(" AM",0,0x0D);	// if am_pm is low i.e. Bit 5 of hours register is 0. Display AM
		}
		
		// Update Time Depending on Button Press
		// Button on PIND6 controls which parameter will be updated i.e. HH,MM,SS or AM/PM
		// Button on PIND7 increments the parameter
		
		if(ButtonPressed(0,PIND,6,100))
		{
			i=0;	                                 // Initialize parameter used to identify which time segment is being modified
													 // i=0. hours value will update
													 // i=1. minutes value will update
													 // i=2. seconds value will update
													 // i=3. AM/PM value will update
													 
			hour    = Update_Value(hour,0,5);		 // hours value updated along with refresh on LCD screen
			minutes = Update_Value(minutes,0,8);	 // minutes value updated along with refresh on LCD screen
			seconds = Update_Value(seconds,0,0x0B);	 // seconds value updated along with refresh on LCD screen
			am_pm   = Update_Value(am_pm,0,0x0D);	 // AM/PM value updated along with refresh on LCD screen
			
			hour = (hour/10 << 4) | (hour % 10) | 0b01000000 | am_pm;	// Format the updated hour value as per BCD specification of DS1307
			twi_Write_Register(0b00000010,hour);						// Update the hour value in DS1307
			
			minutes = (minutes/10 << 4) | (minutes % 10);				// Format the updated minutes value as per BCD specification of DS1307
			twi_Write_Register(0b00000001,minutes);						// Update the minutes value in DS1307
			
			seconds = (seconds/10 << 4) | (seconds % 10);				// Format the update seconds value as per BCD specification of DS1307
			twi_Write_Register(0b00000000,seconds);						// Update the seconds value in DS1307
			
		}
    }
	
}

uint8_t Update_Value(uint8_t Time_Parameter, uint8_t x, uint8_t y)
{
		while(1)
		{
			while((ButtonPressed(1,PIND,7,100)))
			{
				 // Increment hours value
				if(i == 0)
				{
					Time_Parameter ++;
					if(Time_Parameter > 12) Time_Parameter = 1;
							
				}
				// Increment minutes value
				if(i == 1)
				{
					Time_Parameter ++;
					if(Time_Parameter > 59) Time_Parameter =0;
												
				}
				// Increment seconds value
				if(i == 2)
				{
					Time_Parameter ++;
					if(Time_Parameter > 59) Time_Parameter =0;
					
				}	
				if(i == 3)
				{
					Time_Parameter ^= 1 << 5;	 // Toggle Bit 5 to change AM/PM
					if (Time_Parameter)
					{
						Send_a_String(" PM",0,0x0D);
					}
					else
					{
						Send_a_String(" AM",0,0x0D);
					}
				}
				// Following code refreshs the incremented time parameter value on the LCD
				// It is not needed for i=3 as AM/PM is refreshed as soon as AM/PM bit is toggled
				if(i != 3)
				{	
					tens_digit  = (Time_Parameter/10);
					units_digit = (Time_Parameter % 10);
					itoa(tens_digit,LCD_Secs,10);
					Send_a_String(LCD_Secs,x,y);
					itoa(units_digit,LCD_Secs,10);
					Send_a_String(LCD_Secs,x,y+1);
				}
			}
			
			/*if(i != 3)
			{
				Send_a_String("  ",x,y);
				tens_digit  = (Time_Parameter/10);
				units_digit = (Time_Parameter % 10);
				itoa(tens_digit,LCD_Secs,10);
				Send_a_String(LCD_Secs,x,y);
				itoa(units_digit,LCD_Secs,10);
				Send_a_String(LCD_Secs,x,y+1);
			}*/
		    
			
			if(ButtonPressed(0,PIND,6,300))
			{
				i++;						// Button press on PIND6 changes the parameter being updated since 'i' is updated
				return Time_Parameter;		// Return the updated value
			}			
		}
}

