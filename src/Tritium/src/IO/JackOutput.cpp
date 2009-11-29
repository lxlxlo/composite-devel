/*
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * This file is part of Tritium
 *
 * Tritium is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tritium is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "JackClient.hpp"
#include <Tritium/Logger.hpp>
#include <Tritium/IO/JackOutput.hpp>
#ifdef JACK_SUPPORT

#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <cassert>
#include <Tritium/Hydrogen.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/InstrumentList.hpp>
#include <Tritium/Song.hpp>
#include <Tritium/Preferences.hpp>
#include <Tritium/globals.hpp>

namespace Tritium
{

unsigned long jack_server_sampleRate = 0;
jack_nframes_t jack_server_bufferSize = 0;
JackOutput *jackDriverInstance = NULL;

int jackDriverSampleRate( jack_nframes_t nframes, void * /*arg*/ )
{
	QString msg = QString("Jack SampleRate changed: the sample rate is now %1/sec").arg( QString::number( (int) nframes ) );
	INFOLOG( msg );
	jack_server_sampleRate = nframes;
	return 0;
}


int jackDriverBufferSize( jack_nframes_t nframes, void * /*arg*/ )
{
	/* This function does _NOT_ have to be realtime safe.
	 */
	jack_server_bufferSize = nframes;
	return 0;
}

void jackDriverShutdown( void * /*arg*/ )
{
//	jackDriverInstance->deactivate();
	JackClient::get_instance()->clearAudioProcessCallback();
	Hydrogen::get_instance()->raiseError( Hydrogen::JACK_SERVER_SHUTDOWN );
}




JackOutput::JackOutput( JackProcessCallback processCallback )
	: AudioOutput()
{
	INFOLOG( "INIT" );
	__track_out_enabled = Preferences::get_instance()->m_bJackTrackOuts;	// allow per-track output

	jackDriverInstance = this;
	this->processCallback = processCallback;

	track_port_count = 0;

	memset( track_output_ports_L, 0, sizeof(track_output_ports_L) );
	memset( track_output_ports_R, 0, sizeof(track_output_ports_R) );
}



JackOutput::~JackOutput()
{
	INFOLOG( "DESTROY" );
	disconnect();
}



// return 0: ok
// return 1: cannot activate client
// return 2: cannot connect output port
// return 3: Jack server not running
// return 4: output port = NULL
int JackOutput::connect()
{
	INFOLOG( "connect" );
	jack_client_t* client = JackClient::get_instance()->ref();

	JackClient::get_instance()->subscribe((void*)this);
	if ( !client ) {
		Hydrogen::get_instance()->raiseError( Hydrogen::JACK_CANNOT_ACTIVATE_CLIENT );
		return 1;
	}


	bool connect_output_ports = connect_out_flag;
	
	memset( track_output_ports_L, 0, sizeof(track_output_ports_L) );
	memset( track_output_ports_R, 0, sizeof(track_output_ports_R) );

	if ( connect_output_ports ) {
//	if ( connect_out_flag ) {
		// connect the ports
		if ( jack_connect( client, jack_port_name( output_port_1 ), output_port_name_1.toLocal8Bit() ) == 0 &&
		        jack_connect ( client, jack_port_name( output_port_2 ), output_port_name_2.toLocal8Bit() ) == 0 ) {
			return 0;
		}

		INFOLOG( "Could not connect so saved out-ports. Connecting to first pair of in-ports" );
		const char ** portnames = jack_get_ports ( client, NULL, NULL, JackPortIsInput );
		if ( !portnames || !portnames[0] || !portnames[1] ) {
			ERRORLOG( "Could't locate two Jack input port" );
			Hydrogen::get_instance()->raiseError( Hydrogen::JACK_CANNOT_CONNECT_OUTPUT_PORT );
			return 2;
		}
		if ( jack_connect( client, jack_port_name( output_port_1 ), portnames[0] ) != 0 ||
		        jack_connect( client, jack_port_name( output_port_2 ), portnames[1] ) != 0 ) {
			ERRORLOG( "Could't connect to first pair of Jack input ports" );
			Hydrogen::get_instance()->raiseError( Hydrogen::JACK_CANNOT_CONNECT_OUTPUT_PORT );
			return 2;
		}
		free( portnames );
	}
	return 0;
}





void JackOutput::disconnect()
{
	INFOLOG( "disconnect" );
	jack_client_t* client;
	client = JackClient::get_instance()->ref();

	deactivate();

	if (client) {
		if (output_port_1)
			jack_port_unregister(client, output_port_1);
		if (output_port_2)
			jack_port_unregister(client, output_port_2);
		for (int j=0; j<track_port_count; ++j) {
			if (track_output_ports_L[j])
				jack_port_unregister(client, track_output_ports_L[j]);
			if (track_output_ports_R[j])
				jack_port_unregister(client, track_output_ports_R[j]);
		}
	}
	JackClient::get_instance()->unsubscribe((void*)this);
}




void JackOutput::deactivate()
{
	INFOLOG( "[deactivate]" );
	JackClient::get_instance()->clearAudioProcessCallback();
	memset( track_output_ports_L, 0, sizeof(track_output_ports_L) );
	memset( track_output_ports_R, 0, sizeof(track_output_ports_R) );
}

unsigned JackOutput::getBufferSize()
{
	return jack_server_bufferSize;
}

unsigned JackOutput::getSampleRate()
{
	return jack_server_sampleRate;
}

float* JackOutput::getOut_L()
{
	jack_default_audio_sample_t *out = ( jack_default_audio_sample_t * ) jack_port_get_buffer ( output_port_1, jack_server_bufferSize );
	return out;
}

float* JackOutput::getOut_R()
{
	jack_default_audio_sample_t *out = ( jack_default_audio_sample_t * ) jack_port_get_buffer ( output_port_2, jack_server_bufferSize );
	return out;
}



float* JackOutput::getTrackOut_L( unsigned nTrack )
{
	if(nTrack > (unsigned)track_port_count ) return 0;
	jack_port_t *p = track_output_ports_L[nTrack];
	jack_default_audio_sample_t* out = 0;
	if( p ) {
		out = (jack_default_audio_sample_t*) jack_port_get_buffer( p, jack_server_bufferSize);
	}
	return out;
}

float* JackOutput::getTrackOut_R( unsigned nTrack )
{
	if(nTrack > (unsigned)track_port_count ) return 0;
	jack_port_t *p = track_output_ports_R[nTrack];
	jack_default_audio_sample_t* out = 0;
	if( p ) {
		out = (jack_default_audio_sample_t*) jack_port_get_buffer( p, jack_server_bufferSize);
	}
	return out;
}


int JackOutput::init( unsigned /*nBufferSize*/ )
{
	output_port_name_1 = Preferences::get_instance()->m_sJackPortName1;
	output_port_name_2 = Preferences::get_instance()->m_sJackPortName2;

	jack_client_t* client = JackClient::get_instance()->ref();

	// Here, client should either be valid, or NULL.	
	jack_server_sampleRate = jack_get_sample_rate ( client );
	jack_server_bufferSize = jack_get_buffer_size ( client );


	/* tell the JACK server to call `process()' whenever
	   there is work to be done.
	*/
	JackClient::get_instance()->setAudioProcessCallback(this->processCallback);

	#warning "XXX TO-DO: These need to be moved to JackClient"
	JackClient::get_instance()->deactivate();
	/* tell the JACK server to call `srate()' whenever
	   the sample rate of the system changes.
	*/
	jack_set_sample_rate_callback ( client, jackDriverSampleRate, 0 );

	/* tell JACK server to update us if the buffer size
	   (frames per process cycle) changes.
	*/
	jack_set_buffer_size_callback ( client, jackDriverBufferSize, 0 );

	/* tell the JACK server to call `jack_shutdown()' if
	   it ever shuts down, either entirely, or if it
	   just decides to stop calling us.
	*/
	jack_on_shutdown ( client, jackDriverShutdown, 0 );

	JackClient::get_instance()->activate();

	/* create two ports */
	output_port_1 = jack_port_register ( client, "out_L", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 );
	output_port_2 = jack_port_register ( client, "out_R", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 );

	if ( ( output_port_1 == NULL ) || ( output_port_2 == NULL ) ) {
		( Hydrogen::get_instance() )->raiseError( Hydrogen::JACK_ERROR_IN_PORT_REGISTER );
		return 4;
	}


	// clear buffers
//	jack_default_audio_sample_t *out_L = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port_1, jack_server_bufferSize);
//	jack_default_audio_sample_t *out_R = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port_2, jack_server_bufferSize);
//	memset( out_L, 0, nBufferSize * sizeof( float ) );
//	memset( out_R, 0, nBufferSize * sizeof( float ) );

	return 0;
}


/**
 * Make sure the number of track outputs match the instruments in @a song , and name the ports.
 */
void JackOutput::makeTrackOutputs( Song * song )
{

	/// Disable Track Outputs
	if( Preferences::get_instance()->m_bJackTrackOuts == false )
			return;
	///

	InstrumentList * instruments = song->get_instrument_list();
	Instrument * instr;
	int nInstruments = ( int )instruments->get_size();

	// create dedicated channel output ports
	WARNINGLOG( QString( "Creating / renaming %1 ports" ).arg( nInstruments ) );

	for ( int n = nInstruments - 1; n >= 0; n-- ) {
		instr = instruments->get( n );
		setTrackOutput( n, instr );
	}
	// clean up unused ports
	jack_client_t* client = JackClient::get_instance()->ref();
	jack_port_t *p_L, *p_R;
	for ( int n = nInstruments; n < track_port_count; n++ ) {
		p_L = track_output_ports_L[n];
		p_R = track_output_ports_R[n];
		track_output_ports_L[n] = 0;
		jack_port_unregister( client, p_L );
		track_output_ports_R[n] = 0;
		jack_port_unregister( client, p_R );
	}

	track_port_count = nInstruments;
}

/**
 * Give the @a n 'th port the name of @a instr .
 * If the n'th port doesn't exist, new ports up to n are created.
 */
void JackOutput::setTrackOutput( int n, Instrument * instr )
{

	QString chName;
	jack_client_t* client = JackClient::get_instance()->ref();

	if ( track_port_count <= n ) { // need to create more ports
		for ( int m = track_port_count; m <= n; m++ ) {
			chName = QString( "Track_%1_" ).arg( m + 1 );
			track_output_ports_L[m] = jack_port_register ( client, ( chName + "L" ).toLocal8Bit(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 );
			track_output_ports_R[m] = jack_port_register ( client, ( chName + "R" ).toLocal8Bit(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 );
			if ( track_output_ports_R[m] == NULL || track_output_ports_L[m] == NULL ) {
				Hydrogen::get_instance()->raiseError( Hydrogen::JACK_ERROR_IN_PORT_REGISTER );
			}
		}
		track_port_count = n + 1;
	}
	// Now we're sure there is an n'th port, rename it.
	chName = QString( "Track_%1_%2_" ).arg( n + 1 ).arg( instr->get_name() );

	jack_port_set_name( track_output_ports_L[n], ( chName + "L" ).toLocal8Bit() );
	jack_port_set_name( track_output_ports_R[n], ( chName + "R" ).toLocal8Bit() );
}

void JackOutput::setPortName( int nPort, bool bLeftChannel, const QString& sName )
{
//	infoLog( "[setPortName] " + sName );
	jack_port_t *pPort;
	if ( bLeftChannel ) {
		pPort = track_output_ports_L[ nPort ];
	} else {
		pPort = track_output_ports_R[ nPort ];
	}

	int err = jack_port_set_name( pPort, sName.toLocal8Bit() );
	if ( err != 0 ) {
		ERRORLOG( " Error in jack_port_set_name!" );
	}
}

int JackOutput::getNumTracks()
{
//	INFOLOG( "get num tracks()" );
	return track_port_count;
}

};

#endif // JACK_SUPPORT
