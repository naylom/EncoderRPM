/*
	Calculates rpm of encoder every second, supports up to 3 encoders

	V1 	Initial release
	V2  Added ability to calculate PPR from Z (index) channel if connected
*/

#include "Encoder.h"
constexpr pin_size_t AChannelPin = 2;	 // Change this to any unused interrupt pin for your board
constexpr pin_size_t BChannelPin = 3;	 // Change this to any unused interrupt pin for your board
constexpr int16_t	 EncoderPPR	 = 3200; // This code uses the encoder in full quadrature mode, so this number must be 4 * the encoder PPR
constexpr pin_size_t ZChannelPin = 18;	 // Change this to UNUSED if Z channel not connected, else it needs to be an unused interrupt pin
int8_t				 EncoderId;			 // Id for encoder

void				 setup ()
{
	Serial.begin ( 115200 );
	while ( !Serial )
		;
	Serial.println ( "\nEncoder RPM V2" );
	// Add encoder

	EncoderId = TheEncoders.AddEncoder ( EncoderPPR, AChannelPin, BChannelPin, ZChannelPin );
	if ( EncoderId < 0 )
	{
		Serial.print ( "Unable to add encoder" );
		while ( true )
			; // halt
	}
}

void loop ()
{
	static uint32_t ulLastTime	 = 0UL;
	static int32_t	lLastReading = 0UL;
	uint32_t		ulNow		 = millis ();

	if ( ulNow - ulLastTime > 1000 ) // calc result every 1000 ms
	{
		int32_t lReadingNow = TheEncoders.GetReading ( (uint8_t)EncoderId );
		double	rpm			= ( (double)( lReadingNow - lLastReading ) * ( 60.0 * 1000.0 ) ) / (double)( ulNow - ulLastTime ) / (double)EncoderPPR; // steps / ms

		lLastReading		= lReadingNow;
		ulLastTime			= ulNow;

		Serial.print ( "RPM =  " );
		Serial.print ( rpm );

		// Optional check for errors in encoder
		if ( TheEncoders.HasErrors ( EncoderId ) )
		{
			Serial.print ( " Seeing errors on encoder inputs" );
		}
		if ( ZChannelPin != UNUSED )
		{
			Serial.print ( " Calculated (full quadrature) PPR is " );
			Serial.print ( TheEncoders.GetCalculatedPPR ( EncoderId ) );
		}
		Serial.println();
	}
}