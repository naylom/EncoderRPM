#include "Encoder.h"

constexpr pin_size_t AChannelPin = 0; // Change these to any interrupt pin for your board
constexpr pin_size_t BChannelPin = 1;
constexpr int16_t	 EncoderPPR	 = 3200; // This code uses the encoder in full quadrature mode, so this number must be 4 * the encoder PPR

int8_t				 EncoderId; // Id for encoder

void				 setup ()
{
	Serial.begin ( 115200 );
	while ( !Serial )
		;
	Serial.println ( "\nEncoder RPM" );
	// Add encoder

	EncoderId = TheEncoders.AddEncoder ( EncoderPPR, AChannelPin, BChannelPin );
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
		double	rpm			= (double)( lReadingNow - lLastReading ) / (double)( ulNow - ulLastTime ); // steps / ms
		rpm					/= EncoderPPR;															   // revs / ms
		rpm					*= ( 60.0 * 1000.0 );													   // revs per sec
		lLastReading		= lReadingNow;
		ulLastTime			= ulNow;
		Serial.print ( "RPM =  " );
		Serial.println ( rpm );
		// Optional check for errors in encoder
		if ( TheEncoders.HasErrors ( EncoderId ) )
		{
			Serial.println ( "Seeing errors on encoder inputs" );
		}
	}
}