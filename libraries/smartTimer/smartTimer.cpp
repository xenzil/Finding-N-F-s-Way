/*
 * smartTimer.c
 *
 * Created: 13/6/2015 11:38:17 AM
 *  Author: Colin Tan
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "smartTimer.h"

// Array of pointers to object instances, one for each timer
// Timer index
unsigned char tndx;
CSmartTimer *_instanceArray[MAX_TIMERS];

/* RTC Support */
#ifdef RTC

// Test for leap year
uint8_t isLeapYear(uint16_t year)
{
	if(!(year%400))
	return 1;
	
	if(!(year%100))
	return 0;
	
	if(!(year%4))
	return 1;
	
	return 0;
}

// Get last day of a month
uint8_t lastDayOfMonth(uint8_t month, uint16_t year)
{
	switch(month)
	{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
		return 31;
		break;
		
		case 4:
		case 6:
		case 9:
		case 11:
		return 30;
		break;
		
		case 2:
		if(isLeapYear(year))
		return 29;
		else
		return 28;
		
		default:
		return 30;
	}
}
// direction==1 for local->UTC, -1 for UTC->local
void CSmartTimer::convertUTC(int direction)
{
	int8_t hour, min, sec, day, month, year;
	int8_t utc_ofs_hrs=direction*_utc_ofs_hrs;
	int8_t utc_ofs_min=direction*_utc_ofs_min;
	
	min=_min+utc_ofs_min;
	hour=_hour;
	day=_day;
	month=_month;
	year=_year;
	sec=_sec;
	
	if(min>60)
	{
		hour+=1;
		min-=60;
	}
	else
	if(min<0)
	{
		hour-=1;
		min+=60;
	}
	
	hour+=utc_ofs_hrs;
	
	if(hour>24)
	{
		hour-=24;
		day++;
	}
	else
	if(hour < 0)
	{
		hour+=24;
		day--;
	}
	
	if(day<0)
	{
		month--;
		
		if(month<0)
		{
			month=12;
			year--;
		}
		
		day+=lastDayOfMonth(month, year);
	}
	
	if(day > lastDayOfMonth(month, year))
	{
		day -= lastDayOfMonth(month, year);
		month++;
		if(month>12)
		{
			month=1;
			year++;
		}
	}
	
	_epday=day;
	_epmonth=month;
	_epyear=year;
	_ephour=hour;
	_epmin=min;
	_epsec=sec;
}

void CSmartTimer::_setTime(uint8_t day, uint8_t month, uint16_t year, uint8_t hour, uint8_t min, uint8_t sec)
{
	_day=day;
	_month=month;
	_year=year;
	_hour=hour;
	_min=min;
	_sec=sec;
	
	// Convert to UTC
	convertUTC(1);
	
	// Find yday
	uint16_t yday;
	
	const unsigned short int __mon_yday[2][12] =
	{
		/* Normal years.  */
		{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
		/* Leap years.  */
		{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
	};
	
	if(isLeapYear(_year))
	yday=__mon_yday[1][_epmonth-1]+_epday;
	else
	yday=__mon_yday[0][_epmonth-1]+_epday;
	
	_leap1=((_epyear-1969)/4) * 86400;
	_leap2=((_epyear-1)/100)*86400;
	_leap3=((_epyear+299)/400) *86400;
	
	_epoch=_epsec+_epmin*60+_ephour*3600+yday*86400+(_epyear-1970)*31536000 + _leap1 - _leap2 + _leap3;
}

void CSmartTimer::_getTime(uint8_t &day, uint8_t &month, uint16_t &year, uint8_t &hour, uint8_t &min, uint8_t &sec)
{
	day=_day;
	month=_month;
	year=_year;
	hour=_hour;
	min=_min;
	sec=_sec;
}

// Set the number of hours and minutes of your time zone from UTC. If you forget to do this before RTCSetLocalTime
// then your local time will be assumed to be UTC

void CSmartTimer::RTCSetUTCOffset(int8_t hours, int8_t min)
{
	_utc_ofs_hrs=hours;
	_utc_ofs_min=min;
}

// Set the current time
void CSmartTimer::RTCSetLocalTime(uint8_t day, uint8_t month, uint16_t year, uint8_t hour, uint8_t min, uint8_t sec)
{
	_setTime(day, month, year, hour, min, sec);
}

void CSmartTimer::RTCGetLocalTime(uint8_t &day, uint8_t &month, uint16_t &year, uint8_t &hour, uint8_t &min, uint8_t &sec)
{
	_getTime(day, month, year, hour, min, sec);
}

uint64_t CSmartTimer::RTCGetEpochTime()
{
	return _epoch;
}

#endif

// Some constants

void handleInterrupt(unsigned char timer)
{
	CSmartTimer *c=_instanceArray[timer];
	
	// Run through all functions
	for(int i=0; i<c->_callbackCount; i++)
	{
		c->_callbacks[i].count--;
		
		if(!c->_callbacks[i].count)
		{
			c->_callbacks[i].count = c->_callbacks[i].period;
			c->_callbacks[i].callbackPtr();
		}
	}
	
	// Update epoch counter and date if needed
	#ifdef RTC
		c->_secCtr++;
		
		if(c->_secCtr==1000)
		{
			c->_epoch++;
			c->_secCtr=0;
			
			c->_sec++;
			
			if(c->_sec>59)
			{
				c->_sec=0;
				c->_min++;
				
				if(c->_min>59)
				{
					c->_hour++;
					c->_min=0;
					
					if(c->_hour>23)
					{
						c->_day++;
						c->_hour=0;
						
						if(c->_day > lastDayOfMonth(c->_month, c->_year))
						{
							c->_day=1;
							c->_month++;
							
							if(c->_month>12)
							{
								c->_month=1;
								c->_year++;
							}
						}
						
					}
					
				}
					
			}
			
		}
	#endif
}

#if MAX_TIMERS>=2
ISR(TIMER1_OVF_vect)
{
	handleInterrupt(1);
	TCNT1=INIT16;
}
#endif

#if MAX_TIMERS>=3

ISR(TIMER2_OVF_vect)
{
	handleInterrupt(2);
	TCNT2=INIT8;
}

#endif

#if MAX_TIMERS >= 4 && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
ISR(TIMER3_OVF_vect)
{
	handleInterrupt(3);
	TCNT3=INIT16;
}
#endif

#if MAX_TIMERS >= 5 && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
ISR(TIMER4_OVF_vect)
{
	handleInterrupt(4);
	TCNT4=INIT16;
}
#endif

#if MAX_TIMERS >= 6 && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
ISR(TIMER5_OVF_vect)
{
	handleInterrupt(5);
	TCNT5=INIT16;
}
#endif

// Protected methods
void CSmartTimer::_setupTimer()
{
	if(_timerNumber>=MAX_TIMERS)
		return;

	// In each case we set to fast PWM, prescaler of 64. Note that we automatically disconnect the PWM pins,
	// But these are reconnected by analogWrite anyway.	
	switch(_timerNumber)
	{
	/*	case 0: 
		TCCR0A=0b11; // Mode 011 = Fast PWM
		TCCR0B=0b0; // Prescaler set at 0 for now. Setting prescaler triggers the clock.
		TCNT0=6; // Rolls over after 250 counts at default 16MHz clock
		sbi(TIMSK0, TOIE0);
		break;*/
		
#if MAX_TIMERS>=2
		case 1:
		
		if(_timerActive[1])
		{
			// Setting 8-bit fast PWM, mode = 0b0101
			TCCR1A=0b01;
		
			// WGM03=0, WGM02=01 for mode, prescalar = 011 for 64. Here is 0 to switch off the timer.
			TCCR1B=0b01000;
			TCNT1=INIT16; // Based on 8-bit PWM
			sbi(TIMSK1, TOIE1);	
		}
		break;
#endif

#if MAX_TIMERS>=3
		case 2:
		if(_timerActive[2])
		{
			TCCR2A=0b11; // Mode 011 - Fast PWM
			TCCR2B=0b0; // Prescalar of 0b100 for 64 but we set to 0 here.
			TCNT2=INIT8;
			sbi(TIMSK2, TOIE2);	
		}
		break;
#endif

#if MAX_TIMERS>=4  && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
		case 3:
		
		if(_timerActive[3])
		{
			// Setting 8-bit fast PWM, mode = 0b0101
			TCCR3A=0b01;
				
			// WGM03=0, WGM02=01 for mode, prescalar = 011 for 64. Here is 0 to switch off the timer.
			TCCR3B=0b01000;
			TCNT3=INIT16; // Based on 8-bit PWM
			sbi(TIMSK3, TOIE3);

		}
		break;
#endif

#if MAX_TIMERS>=5  && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
		case 4:

		if(_timerActive[4])
		{
			// Setting 8-bit fast PWM, mode = 0b0101
			TCCR4A=0b01;
				
			// WGM03=0, WGM02=01 for mode, prescalar = 011 for 64. Here is 0 to switch off the timer.
			TCCR4B=0b01000;
			TCNT4=INIT16;
			sbi(TIMSK4, TOIE4);
		}
		break;
#endif

#if MAX_TIMERS>=6  && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
		case 5:
		
			if(_timerActive[5])
			{
				// Setting 8-bit fast PWM, mode = 0b0101
				TCCR5A=0b01;
						
				// WGM03=0, WGM02=01 for mode, prescalar = 011 for 64. Here is 0 to switch off the timer.
				TCCR5B=	0b01000;
				TCNT5=INIT16;
				sbi(TIMSK5, TOIE5);				
			}
		break;
#endif


	}
}

void CSmartTimer::_startTimer()
{
	
//	TCCR0B=0b11; // Prescalar of 64
	#if MAX_TIMERS >=2
		if(_timerActive[1])
			TCCR1B=0b01011; // Prescalar of 011 for 64
	#endif

	#if MAX_TIMERS >=3
		if(_timerActive[2])
			TCCR2B=0b100; 
	#endif

	#if MAX_TIMERS >= 4 && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
		if(_timerActive[3])
			TCCR3B=0b01011; // Prescalar of 011 for 64
	#endif

	#if MAX_TIMERS >= 5 && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
		if(_timerActive[4])
			TCCR4B=0b01011; // Prescalar of 011 for 64
	#endif

	#if MAX_TIMERS >= 6 && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
		if(_timerActive[5])
			TCCR5B=0b01011; // Prescalar of 011 for 64
	#endif
	
	sregVal=SREG;
	sei();
}

void CSmartTimer::_stopTimer()
{
	SREG=sregVal;
	
//	TCCR0B=0b0;
	
	#if MAX_TIMERS >=2
		TCCR1B=0b01000;
	#endif

	#if MAX_TIMERS >=3
		TCCR2B=0b0;
	#endif

	#if MAX_TIMERS >= 4 && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
		TCCR3B=0b01000;
	#endif

	#if MAX_TIMERS >= 5 && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
		TCCR4B=0b01000;
	#endif

	#if MAX_TIMERS >= 6 && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
		TCCR5B=0b01000;
	#endif
}

// Public methods

// Constructor. Declares which timer to control. Do not control the same timer
// from two different objects!

CSmartTimer::CSmartTimer(unsigned char whichTimer)
{
	memset(_timerActive, 0, sizeof(_timerActive));
	
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	if(whichTimer < 6)
#else
	if(whichTimer < 3)
#endif
	{
		_timerNumber=whichTimer;
		tndx=whichTimer;
		_timerActive[whichTimer]=1;
		_instanceArray[tndx]=this;		
	}
	else
		_timerNumber=-1;
	
	if(_timerNumber >=0)	
	{
		// Initialize the epoch counter and second counter if supporting RTC
		#ifdef RTC
		_secCtr=0;
		_epoch=0;
		
		#endif
		_callbackCount=0;
		// Initialize the timer
		_setupTimer();		
	}	
}
		
// Attach callbacks, and period to call the callback in milliseconds. You can attach up to
// four callbacks. Returns 0 on success, -1 on failure.
int CSmartTimer::attachCallback(void (*callbackPtr)(void), unsigned int ms)
{
	if(_timerNumber<0 || _callbackCount>=MAX_CALLBACKS)
		return -1;
		
	_callbacks[_callbackCount].callbackPtr=callbackPtr;
	_callbacks[_callbackCount].period=ms;
	_callbacks[_callbackCount].count=ms;
	_callbackCount++;
	
	return 0;
}
		
// Start the timer
void CSmartTimer::startTimer()
{
	_startTimer();
}
		
// Stop the timer
void CSmartTimer::stopTimer()
{
	_stopTimer();
}

/*// Destructor
CSmartTimer::~CSmartTimer();
{
	
}*/
