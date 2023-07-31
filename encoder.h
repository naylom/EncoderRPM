#pragma once
/*
	encoder.h

	Classes to encapsulate a full quadrature encoder

	Encoders class holds a list of all encoders (max is 3)
	Encoder class represents one encoder

	This is designed to work with an encoder in full quadrature mode ie needs a and b channel connected to arduino
*/
#include <arduino.h>

#ifndef NOT_A_PIN
	#define NOT_A_PIN ( 255 )
#endif
#ifndef pin_size_t
	#define pin_size_t uint8_t
#endif
#ifndef UNUSED
constexpr pin_size_t UNUSED = 255;
#endif

typedef void ( *InterruptCallback ) ( void );
#define OUTPUT_STEPS	 1
#define OUTPUT_SELECTION OUTPUT_STEPS
constexpr uint8_t MAX_ENCODERS = 3; // Allow for X, Y and Z encoder support

class Encoder
{
	private:
		volatile uint32_t m_ulFwdCount;	   // volatile counter of A channel positive direction pulses
		volatile uint32_t m_ulRevCount;	   // volatile counter of A channel negative direction pulses
		volatile int32_t  m_lLastNetCount; // Last sum of FwdCount - RevCount
		volatile int32_t  m_lPulsesAtIndex;
		volatile uint32_t m_ulCalculatedPPR;
		uint32_t		  m_ulLastNetCountTime; // time in milliseconds last net count was created
		volatile uint32_t m_ulErrCount;			// Number of times error state seen
		uint16_t		  m_uiPPR;				// Full quadrature value, ie for a 800 PPR encoder this should be set to 800 * 4
		pin_size_t		  m_bChannelAPin;
		pin_size_t		  m_bChannelBPin;
		pin_size_t		  m_bChannelZPin;
		bool			  m_bFirstZPulse; // true until we see a pulse on Z channel
		int8_t			  m_bLastState;	  // 00b, 01b, 10b or 11b
		volatile bool	  m_bInError;	  // true if sequential encoder reads indicate skipped reading

	public:
		Encoder ( int16_t iPPR, pin_size_t aChannelPin, pin_size_t bChannelPin, pin_size_t m_zChannelBPin, uint8_t index );
		void	 ProcessPulse ();
		void	 ProcessZPulse ();
		int32_t	 GetReading ();
		void	 SetState ( int8_t newState );
		int8_t	 GetState ();
		uint32_t GetCalculatedPPR ();
		void	 IncRevCount ();
		void	 IncFwdCount ();
		void	 IncErrCount ();
		bool	 IsInUse ();
		bool	 IsInError ();
};

class Encoders
{
	public:
		int8_t	 AddEncoder ( int16_t iPPR, pin_size_t aChannelPin, pin_size_t bChannelPin, pin_size_t zChannelPin );
		Encoder &GetEncoder ( uint8_t index );
		uint32_t GetReading ( uint8_t index );
		bool	 HasErrors ( uint8_t index );
		uint32_t GetCalculatedPPR ( uint8_t index );

	private:
		int8_t	 m_iNumEncoders = 0;
		Encoder *m_listEncoder [ MAX_ENCODERS ];
};

extern Encoders TheEncoders;
