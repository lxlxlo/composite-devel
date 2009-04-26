/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <hydrogen/IO/JackOutput.h>
#ifdef JACK_SUPPORT

#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <cassert>
#include <hydrogen/hydrogen.h>
#include <hydrogen/instrument.h>
#include <hydrogen/Song.h>
#include <hydrogen/Preferences.h>
#include <hydrogen/globals.h>

#ifdef LASH_SUPPORT
#include <hydrogen/LashClient.h>
#endif

namespace H2Core
{

unsigned long jack_server_sampleRate = 0;
jack_nframes_t jack_server_bufferSize = 0;
JackOutput *jackDriverInstance = NULL;

int jackDriverSampleRate( jack_nframes_t nframes, void *arg )
{
	UNUSED( arg );
	QString msg = QString("Jack SampleRate changed: the sample rate is now %1/sec").arg( QString::number( (int) nframes ) );
	_INFOLOG( msg );
	jack_server_sampleRate = nframes;
	return 0;
}




void jackDriverShutdown( void *arg )
{
	UNUSED( arg );
//	jackDriverInstance->deactivate();
	jackDriverInstance->client = NULL;
	Hydrogen::get_instance()->raiseError( Hydrogen::JACK_SERVER_SHUTDOWN );
}




JackOutput::JackOutput( JackProcessCallback processCallback )
		: AudioOutput( "JackOutput" )
{
	INFOLOG( "INIT" );
	__track_out_enabled = Preferences::getInstance()->m_bJackTrackOuts;	// allow per-track output

	jackDriverInstance = this;
	this->processCallback = processCallback;

	track_port_count = 0;
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

	if ( jack_activate ( client ) ) {
		Hydrogen::get_instance()->raiseError( Hydrogen::JACK_CANNOT_ACTIVATE_CLIENT );
		return 1;
	}


	bool connect_output_ports = connect_out_flag;
	
#ifdef LASH_SUPPORT
	if ( Preferences::getInstance()->useLash() ){
		LashClient* lashClient = LashClient::getInstance();
		if (lashClient && lashClient->isConnected())
		{
	//		infoLog("[LASH] Sending Jack client name to LASH server");
			lashClient->sendJackClientName();
			
			if (!lashClient->isNewProject())
			{
				connect_output_ports = false;
			}
		}
	}
#endif
	
	if ( connect_output_ports ) {
//	if ( connect_out_flag ) {
		// connect the ports
		if ( jack_connect( client, jack_port_name( output_port_1 ), output_port_name_1.toAscii() ) == 0 &&
		        jack_connect ( client, jack_port_name( output_port_2 ), output_port_name_2.toAscii() ) == 0 ) {
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

	deactivate();
	jack_client_t *oldClient = client;
	client = NULL;
	if ( oldClient ) {
		INFOLOG( "calling jack_client_close" );
		int res = jack_client_close( oldClient );
		if ( res ) {
			ERRORLOG( "Error in jack_client_close" );
			// FIXME: raise exception
		}
	}
	client = NULL;
}




void JackOutput::deactivate()
{
	INFOLOG( "[deactivate]" );
	if ( client ) {
		INFOLOG( "calling jack_deactivate" );
		int res = jack_deactivate( client );
		if ( res ) {
			ERRORLOG( "Error in jack_deactivate" );
		}
	}
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
	jack_default_audio_sample_t *out = ( jack_default_audio_sample_t * ) jack_port_get_buffer ( track_output_ports_L[nTrack], jack_server_bufferSize );
	return out;
}

float* JackOutput::getTrackOut_R( unsigned nTrack )
{
	jack_default_audio_sample_t *out = ( jack_default_audio_sample_t * ) jack_port_get_buffer ( track_output_ports_R[nTrack], jack_server_bufferSize );
	return out;
}


#define CLIENT_FAILURE(msg) {						\
		ERRORLOG("Could not connect to JACK server (" msg ")"); \
		if (client) {						\
			ERRORLOG("...but JACK returned a non-null pointer?"); \
			(client) = 0;					\
		}							\
		if (tries) ERRORLOG("...trying again.");		\
	}


#define CLIENT_SUCCESS(msg) {				\
		assert(client);				\
		INFOLOG(msg);				\
		tries = 0;				\
	}

int JackOutput::init( unsigned /*nBufferSize*/ )
{

	output_port_name_1 = Preferences::getInstance()->m_sJackPortName1;
	output_port_name_2 = Preferences::getInstance()->m_sJackPortName2;

	QString sClientName = "Hydrogen";
	jack_status_t status;
	int tries = 2;  // Sometimes jackd doesn't stop and start fast enough.
	while ( tries > 0 ) {
		--tries;
		client = jack_client_open(
			sClientName.toAscii(),
			JackNullOption,
			&status);
		switch(status) {
		case JackFailure:
			CLIENT_FAILURE("unknown error");
			break;
		case JackInvalidOption:
			CLIENT_FAILURE("invalid option");
			break;
		case JackNameNotUnique:
			if (client) {
				sClientName = jack_get_client_name(client);
				CLIENT_SUCCESS(QString("Jack assigned the client name '%1'")
					       .arg(sClientName));
			} else {
				CLIENT_FAILURE("name not unique");
			}
			break;
		case JackServerStarted:
			CLIENT_SUCCESS("JACK Server started for Hydrogen.");
			break;
		case JackServerFailed:
			CLIENT_FAILURE("unable to connect");
			break;
		case JackServerError:
			CLIENT_FAILURE("communication error");
			break;
		case JackNoSuchClient:
			CLIENT_FAILURE("unknown client type");
			break;
		case JackLoadFailure:
			CLIENT_FAILURE("can't load internal client");
			break;
		case JackInitFailure:
			CLIENT_FAILURE("can't initialize client");
			break;
		case JackShmFailure:
			CLIENT_FAILURE("unable to access shared memory");
			break;
		case JackVersionError:
			CLIENT_FAILURE("client/server protocol version mismatch");
		default:
			if (status) {
				ERRORLOG("Unknown status with JACK server.");
				if (client) {
					CLIENT_SUCCESS("Client pointer is *not* null..."
						       " assuming we're OK");
				}
			} else {
				CLIENT_SUCCESS("Connected to JACK server");
			}				
		}
	}

	if (client == 0) {
	    return -1;
	}

	// Here, client should either be valid, or NULL.	
	jack_server_sampleRate = jack_get_sample_rate ( client );
	jack_server_bufferSize = jack_get_buffer_size ( client );


	/* tell the JACK server to call `process()' whenever
	   there is work to be done.
	*/
	jack_set_process_callback ( client, this->processCallback, 0 );


	/* tell the JACK server to call `srate()' whenever
	   the sample rate of the system changes.
	*/
	jack_set_sample_rate_callback ( client, jackDriverSampleRate, 0 );


	/* tell the JACK server to call `jack_shutdown()' if
	   it ever shuts down, either entirely, or if it
	   just decides to stop calling us.
	*/
	jack_on_shutdown ( client, jackDriverShutdown, 0 );


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

#ifdef LASH_SUPPORT
	if ( Preferences::getInstance()->useLash() ){
		LashClient* lashClient = LashClient::getInstance();
		if (lashClient->isConnected())
		{
			lashClient->setJackClientName(sClientName.toStdString());
		}
	}
#endif

	return 0;
}


/**
 * Make sure the number of track outputs match the instruments in @a song , and name the ports.
 */
void JackOutput::makeTrackOutputs( Song * song )
{

	/// Disable Track Outputs
	if( Preferences::getInstance()->m_bJackTrackOuts == false )
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
	for ( int n = nInstruments; n < track_port_count; n++ ) {
		jack_port_unregister( client, track_output_ports_L[n] );
		jack_port_unregister( client, track_output_ports_R[n] );
		track_output_ports_L[n] = NULL;
		track_output_ports_R[n] = NULL;
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

	if ( track_port_count <= n ) { // need to create more ports
		for ( int m = track_port_count; m <= n; m++ ) {
			chName = QString( "Track_%1_" ).arg( m + 1 );
			track_output_ports_L[m] = jack_port_register ( client, ( chName + "L" ).toAscii(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 );
			track_output_ports_R[m] = jack_port_register ( client, ( chName + "R" ).toAscii(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 );
			if ( track_output_ports_R[m] == NULL || track_output_ports_L[m] == NULL ) {
				Hydrogen::get_instance()->raiseError( Hydrogen::JACK_ERROR_IN_PORT_REGISTER );
			}
		}
		track_port_count = n + 1;
	}
	// Now we're sure there is an n'th port, rename it.
	chName = QString( "Track_%1_%2_" ).arg( n + 1 ).arg( instr->get_name() );

	jack_port_set_name( track_output_ports_L[n], ( chName + "L" ).toAscii() );
	jack_port_set_name( track_output_ports_R[n], ( chName + "R" ).toAscii() );
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

	int err = jack_port_set_name( pPort, sName.toAscii() );
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
