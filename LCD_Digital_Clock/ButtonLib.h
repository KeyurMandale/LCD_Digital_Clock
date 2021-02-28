/***********************************Software _Debouncing******************************************/
#ifndef ButtonLib
#define ButtonLib

#include <avr/io.h>

char ButtonPressed(int ButtonNo, char Portname, char PinNo, int Confidence_Level);

char Pressed[NoofButtons];
int Pressed_confidence[NoofButtons];
int Released_confidence[NoofButtons];

char ButtonPressed(int ButtonNo, char Portname, char PinNo, int Confidence_Level)
{
	if(bit_is_clear(Portname, PinNo))
	{
		Pressed_confidence[ButtonNo] ++;
		Released_confidence[ButtonNo] = 0 ;
		if(Pressed_confidence[ButtonNo] > Confidence_Level)
		{
			if(Pressed[ButtonNo] == 0)
			{
				Pressed[ButtonNo] = 1;
				return 1;
			}
			Pressed_confidence[ButtonNo] = 0;
		}
		
	}
	else
	{
		Released_confidence[ButtonNo] ++;
		Pressed_confidence[ButtonNo] = 0;
		if(Released_confidence[ButtonNo] > Confidence_Level)
		{
			Pressed[ButtonNo] = 0;
			Released_confidence[ButtonNo] = 0;
		}
	}
	return 0;
}

#endif