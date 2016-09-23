/*
 * smartTimer.h
 *
 * Created: 13/6/2015 11:38:36 AM
 *  Author: Colin Tan
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#ifndef SMARTTIMER_H_
#define SMARTTIMER_H_

// The SBI and CBI macros
#ifndef sbi
	#define sbi(port, bit) (port) |= (1 << (bit));
#endif

#ifndef cbi
	#define cbi(port, bit) (port) &= ~(1 << (bit));
#endif

// In Arduino these would have been defined
#ifndef F_CPU
#define F_CPU 16000000
#endif

#ifndef __AVR_ATmega2560__
#define __AVR_ATmega2560__
#endif

// Uncomment if you want real-time clock support. 
// WARNING: This adds 3,000 bytes to your executable!
//#define RTC

// Number of timers you will be using. You can then use timers 1 to NUM_TIMERS
#define NUM_TIMERS			5

// Maximum number of callbacks per timer you are declaring
#define MAX_CALLBACKS		3

//Jiffy time in milliseconds
#define JIFFYMS				1.0

// Prescaler. Note we choose prescaler of 64 for all timers. Changing this DOES NOT change 
// the actual prescalers set on the timers. To do this you must modify the initialize code
// in smartTimer.cpp
//
// This one is solely for calculating INIT8 and INIT16, the initial values to be loaded onto the
// respective TCNT registers.

#define PRESCALER			64

// Maximum number of timers you will be using. You can use timers 1 to MAX_TIMERS-1. We always +1 to the number
// of timers we want, to take into account Timer0 which is reserved by Arduino
#define MAX_TIMERS			(NUM_TIMERS+1)

// 16 bit and 8 bit rollover values
#define ROLL8	256
#define ROLL16	256

// 8-bit initial count to be loaded onto TCNT2
#define INIT8 (ROLL8 - (int)(JIFFYMS/((PRESCALER * 1000.0)/F_CPU)))

// 16-bit initial count to be loaded onto TCNT{1,3,4,5}
#define INIT16 (ROLL16 - (int)(JIFFYMS/((PRESCALER * 1000.0)/F_CPU)))

// Timer definitions
enum
{
	TIMER0_IS_UNAVAILABLE, // Unavailable for use. Reserved by Arduino
	STIMER1, 
	STIMER2,
	STIMER3,
	STIMER4,
	STIMER5	
};

// ISR prototypes
//extern "C" void TIMER0_OVF_vect(void) __attribute__ ((signal));

#if MAX_TIMERS >= 2
extern "C" void TIMER1_OVF_vect(void) __attribute__ ((signal));
#endif

#if MAX_TIMERS >= 3
extern "C" void TIMER2_OVF_vect(void) __attribute__ ((signal));
#endif

#if MAX_TIMERS >= 4 && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
extern "C" void TIMER3_OVF_vect(void) __attribute__ ((signal));
#endif

#if MAX_TIMERS >= 5 && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
extern "C" void TIMER4_OVF_vect(void) __attribute__ ((signal));
#endif

#if MAX_TIMERS >= 6 && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
extern "C" void TIMER5_OVF_vect(void) __attribute__ ((signal));
#endif

typedef struct _callbackInfo
{
	void (*callbackPtr)(void);
	unsigned int period;
	unsigned int count;
} TCallbackInfo;

void handleInterrupt(unsigned char);

class CSmartTimer
{
	protected:
		char _timerNumber; 
		unsigned char _callbackCount;
		// Callback pointers
		TCallbackInfo _callbacks[MAX_CALLBACKS];

		// Record which timer is active
		unsigned char _timerActive[MAX_TIMERS];

		// Storing SREG
		char sregVal;
		
		friend void handleInterrupt(unsigned char);
		
	//	friend void TIMER0_OVF_vect(void);

#if MAX_TIMERS >=2
		friend void TIMER1_OVF_vect(void);
#endif

#if MAX_TIMERS >=3
	friend void TIMER2_OVF_vect(void);
#endif

#if MAX_TIMERS >= 4 && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
	friend void TIMER3_OVF_vect(void);
#endif

#if MAX_TIMERS >= 5 && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
friend void TIMER4_OVF_vect(void);
#endif

#if MAX_TIMERS >= 6 && (defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__))
friend void TIMER5_OVF_vect(void);
#endif

	void _setupTimer();
	void _startTimer();
	void _stopTimer();
	
	// Real time clock support
	#ifdef RTC
		uint16_t _secCtr; // This counter keeps track of seconds. It is incremented every ms and rolls over to 0 when it reachers 999
		uint64_t _epoch;
		int8_t _utc_ofs_hrs, _utc_ofs_min;
		uint8_t _day, _month, _hour, _min, _sec;
		uint16_t _year;
		uint8_t _epday, _epmonth, _ephour, _epmin, _epsec;
		uint16_t _epyear;
		uint64_t _leap1, _leap2, _leap3;
		
		void convertUTC(int direction);
		
		void _setTime(uint8_t day, uint8_t month, uint16_t year, uint8_t hour, uint8_t min, uint8_t second);
		void _getTime(uint8_t &day, uint8_t &month, uint16_t &year, uint8_t &hour, uint8_t &min, uint8_t &second);
	#endif

	public:
	
		// Constructor. Declares which timer to control. Do not control the same timer
		// from two different objects!
		
		CSmartTimer(unsigned char whichTimer);
		
		// Attach callbacks, and period to call the callback in milliseconds. You can attach up to
		// four callbacks. Returns 0 on success, -1 on failure.
		// IMPORTANT: Total execution time of all callbacks MUST BE under 1 ms.
		int attachCallback(void (*callbackPtr)(void), unsigned int ms);
		
		// Start the timer
		void startTimer();
		
		// Stop the timer
		void stopTimer();
		
		// RTC Support
		#ifdef RTC
			// Set the number of hours and minutes of your time zone from UTC. If you forget to do this before RTCSetLocalTime 
			// then your local time will be assumed to be UTC
			
			void RTCSetUTCOffset(int8_t hours, int8_t min);
			// Set the current time
			void RTCSetLocalTime(uint8_t day, uint8_t month, uint16_t year, uint8_t hour, uint8_t min, uint8_t second);
			void RTCGetLocalTime(uint8_t &day, uint8_t &month, uint16_t &year, uint8_t &hour, uint8_t &min, uint8_t &second);
			uint64_t RTCGetEpochTime();
		#endif
		// Destructor
	//	~CSmartTimer();
};

#endif /* SMARTTIMER_H_ */