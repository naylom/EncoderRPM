#include "encoder.h"

/*
	encoder.cpp

	Contains implementation of encoder.h

*/
// #define MKR_WIFI_1010
#define MAKESTATE( ASTATE, BSTATE ) ( ASTATE | ( 0b10 & ( BSTATE << 1 ) ) ) // sets two bits to represent state of A and B Channel eg 0b00000011 is both HIGH
#define ERR_STATE					-9

// The following state table represents what to do based on the last and current state of the A and B Channels
// The two dimensional table is indexed by converting the state of the two channels into a value 0 to 3 inclusive where LOW, LOW = 0 (00b)and HIGH, HIGH = 3 (11b)
// then the action to take is found from iEncoderStateTable [ LastState ][ Newstate ]
// The action is to change the count by -1, 0, 1 or if ERR_STATE we have an unexpected change where both channels changed state and hence we declare the encoder in error
int8_t iEncoderStateTable [ 4 ][ 4 ] = {
	{0,		  -1,		 1,			ERR_STATE},
	{ 1,		 0,			ERR_STATE, -1		  },
	{ -1,		  ERR_STATE, 0,			1		  },
	{ ERR_STATE, 1,			-1,		0		  }
};

// ISR to process signal change on encoder A or B channel
void EncoderISR ( void *param )
{
	TheEncoders.GetEncoder ( ( *(uint8_t *)param ) ).ProcessPulse ();
}

// ISR to process signal change on encoder A or B channel for encoder 0
void EncoderISR0 ()
{
	TheEncoders.GetEncoder ( 0 ).ProcessPulse ();
}

void EncoderISR0_Z ()
{
	TheEncoders.GetEncoder ( 0 ).ProcessZPulse ();
}

// ISR to process signal change on encoder A or B channel for encoder 1
void EncoderISR1 ()
{
	TheEncoders.GetEncoder ( 1 ).ProcessPulse ();
}

void EncoderISR1_Z ()
{
	TheEncoders.GetEncoder ( 1 ).ProcessZPulse ();
}

// ISR to process signal change on encoder A or B channel for encoder 2
void EncoderISR2 ()
{
	TheEncoders.GetEncoder ( 2 ).ProcessPulse ();
}

void EncoderISR2_Z ()
{
	TheEncoders.GetEncoder ( 2 ).ProcessZPulse ();
}

/// @brief Initialises an encoder object
/// @param iPPR how many pulses per revolution in full quadrature mode is for a 200PPR encoder this should be 200 * 4 = 800
/// @param aChannelPin Pin number connected to A channel of encoder
/// @param bChannelPin Pin number connected to B channel of encoder
/// @param zChannelPin Pin number connected to Z channel of encoder
/// @param index Which encoder (of three is being created)
Encoder::Encoder ( int16_t iPPR, pin_size_t aChannelPin, pin_size_t bChannelPin, pin_size_t zChannelPin, uint8_t index )
{
	m_uiPPR				 = iPPR;
	m_bChannelAPin		 = aChannelPin;
	m_bChannelBPin		 = bChannelPin;
	m_bChannelZPin		 = zChannelPin;
	m_bInError			 = false;
	m_bFirstZPulse		 = true;
	m_lLastNetCount		 = 0L;
	m_lPulsesAtIndex	 = 0L;
	m_ulCalculatedPPR	 = 0UL;
	m_ulLastNetCountTime = 0UL;
	m_ulErrCount		 = 0UL;
	m_ulFwdCount		 = 0UL;
	m_ulRevCount		 = 0UL;
	if ( m_bChannelAPin != UNUSED && m_bChannelBPin != UNUSED )
	{
		pinMode ( m_bChannelAPin, INPUT_PULLUP ); // internal pullup input pin
		pinMode ( m_bChannelBPin, INPUT_PULLUP ); // internal pullup input pin
		m_bLastState = MAKESTATE ( digitalRead ( m_bChannelAPin ), digitalRead ( m_bChannelBPin ) );
		if ( m_bChannelZPin != UNUSED )
		{
			pinMode ( m_bChannelZPin, INPUT_PULLUP ); // internal pullup input pin
		}
		switch ( index )
		{
			case 0:
				attachInterrupt ( digitalPinToInterrupt ( aChannelPin ), EncoderISR0, CHANGE );
				attachInterrupt ( digitalPinToInterrupt ( bChannelPin ), EncoderISR0, CHANGE );
				if ( m_bChannelZPin != UNUSED )
				{
					attachInterrupt ( digitalPinToInterrupt ( m_bChannelZPin ), EncoderISR0_Z, FALLING );
				}
				break;
			case 1:
				attachInterrupt ( digitalPinToInterrupt ( aChannelPin ), EncoderISR1, CHANGE );
				attachInterrupt ( digitalPinToInterrupt ( bChannelPin ), EncoderISR1, CHANGE );
				if ( m_bChannelZPin != UNUSED )
				{
					attachInterrupt ( digitalPinToInterrupt ( m_bChannelZPin ), EncoderISR1_Z, FALLING );
				}
				break;
			case 2:
				attachInterrupt ( digitalPinToInterrupt ( aChannelPin ), EncoderISR2, CHANGE );
				attachInterrupt ( digitalPinToInterrupt ( bChannelPin ), EncoderISR2, CHANGE );
				if ( m_bChannelZPin != UNUSED )
				{
					attachInterrupt ( digitalPinToInterrupt ( m_bChannelZPin ), EncoderISR2_Z, FALLING );
				}
				break;
		}
	}
}

//
// Called by interrupt routine when an encoder channel signals a change
//
void Encoder::ProcessPulse ()
{
	bool   bResult	  = false;
	int8_t bThisState = MAKESTATE ( digitalRead ( m_bChannelAPin ), digitalRead ( m_bChannelBPin ) );
	// use last state and this state of A and B channels to determine direction and up relevant count
	switch ( iEncoderStateTable [ GetState () ][ bThisState ] )
	{
		case -1:
			IncRevCount ();
			break;

		case 0: // No change, do nothing
			break;

		case 1:
			IncFwdCount ();
			break;

		default: // Error, both channels have changed state in consecutive interrupts - shouldn't happen - overloaded?
			bResult = true;
			IncErrCount ();
			break;
	}
	SetState ( bThisState );
	if ( m_bInError == false )
	{
		m_bInError = bResult;
	}
}

//
// Called by interrupt routine when an encoder index (Z) channel signal falls
//
void Encoder::ProcessZPulse ()
{
	// check if first time we have seen a Z channel signal
	if ( m_bFirstZPulse == true )
	{
		m_bFirstZPulse	 = false;		
	}
	else
	{
		// Subsequent index pulse
		m_ulCalculatedPPR = abs ( m_lLastNetCount - m_lPulsesAtIndex );
	}
	// save encoder pulses at this point
	m_lPulsesAtIndex = m_lLastNetCount;
}

inline void Encoder::SetState ( int8_t newState )
{
	m_bLastState = newState;
}

inline int8_t Encoder::GetState ()
{
	return m_bLastState;
}

bool Encoder::IsInError ()
{
	bool result = m_bInError;
	m_bInError	= false;
	return result;
}

inline void Encoder::IncRevCount ()
{
	m_ulRevCount++;
	m_lLastNetCount--;
}

inline void Encoder::IncFwdCount ()
{
	m_ulFwdCount++;
	m_lLastNetCount++;
}

int32_t Encoder::GetReading ()
{
	return m_lLastNetCount;
}

inline void Encoder::IncErrCount ()
{
	m_ulErrCount++;
}

/// @brief Returns the number of puleses on A and B channels between to Z channel (index) pulses
/// @return uint32_t value
uint32_t Encoder::GetCalculatedPPR()
{
	return m_ulCalculatedPPR;
}

/// @brief Checks if the encoder pins are both valid
/// @return true if both valid
inline bool Encoder::IsInUse ()
{
	return ( m_bChannelAPin != NOT_A_PIN && m_bChannelBPin != NOT_A_PIN );
}

// @brief Gets reference to the requested encoder object
/// @param index id of encoder, must be >= 0  and < MAX_ENCODERS
/// @return reference to encoder object
inline Encoder &Encoders::GetEncoder ( uint8_t index )
{
	return *m_listEncoder [ index % m_iNumEncoders ];
}

uint32_t Encoders::GetReading ( uint8_t index )
{
	return GetEncoder ( index ).GetReading ();
}

/// @brief Attempts to add a new encoder
/// @param iPPR how many pulses per revolution in full quadrature mode is for a 200PPR encoder this should be 200 * 4 = 800
/// @param aChannelPin Pin number connected to A channel of encoder, NOT_A_PIN if encoder not in use
/// @param bChannelPin Pin number connected to B channel of encoder, NOT_A_PIN if encoder not in use
int8_t Encoders::AddEncoder ( int16_t iPPR, pin_size_t aChannelPin, pin_size_t bChannelPin, pin_size_t zChannelPin )
{
	bool Result = -1;
	if ( m_iNumEncoders < MAX_ENCODERS )
	{
		// Have space so create a new encoder
		Encoder *newEncoder				 = new Encoder ( iPPR, aChannelPin, bChannelPin, zChannelPin, m_iNumEncoders );
		// Save it
		m_listEncoder [ m_iNumEncoders ] = newEncoder;
		Result							 = m_iNumEncoders++;
	}
	return Result;
}

bool Encoders::HasErrors ( uint8_t index )
{
	return GetEncoder ( index ).IsInError ();
}

uint32_t Encoders::GetCalculatedPPR ( uint8_t index )
{
	return GetEncoder ( index ).GetCalculatedPPR();
}
Encoders TheEncoders;
