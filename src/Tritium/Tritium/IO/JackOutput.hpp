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

#ifndef TRITIUM_JACKOUTPUT_HPP
#define TRITIUM_JACKOUTPUT_HPP

#include <Tritium/IO/AudioOutput.hpp>
#include <Tritium/IO/NullDriver.hpp>

// check if jack support is enabled
#ifdef JACK_SUPPORT

#include <jack/jack.h>

#include <Tritium/globals.hpp>
#include <Tritium/Hydrogen.hpp>



namespace Tritium
{

class Song;
class Instrument;
class JackClient;

///
/// Jack (Jack Audio Connection Kit) server driver.
///
class JackOutput : public AudioOutput
{
public:
	JackOutput( JackClient* parent, JackProcessCallback processCallback );
	~JackOutput();

	int connect();
	void disconnect();
	void deactivate();
	unsigned getBufferSize();
	unsigned getSampleRate();
	int getNumTracks();

	void setPortName( int nPort, bool bLeftChannel, const QString& sName );
	void makeTrackOutputs( Song * );
	void setTrackOutput( int, Instrument * );

	void setConnectDefaults( bool flag ) {
		connect_out_flag = flag;
	}
	bool getConnectDefaults() {
		return connect_out_flag;
	}

	float* getOut_L();
	float* getOut_R();
	float* getTrackOut_L( unsigned nTrack );
	float* getTrackOut_R( unsigned nTrack );

	int init( unsigned bufferSize );

private:
	Tritium::Hydrogen *m_pEngine;
	Tritium::JackClient *m_jack_client;

	JackProcessCallback processCallback;
	jack_port_t *output_port_1;
	jack_port_t *output_port_2;
	QString output_port_name_1;
	QString output_port_name_2;
	int track_port_count;
	jack_port_t *track_output_ports_L[MAX_INSTRUMENTS];
	jack_port_t *track_output_ports_R[MAX_INSTRUMENTS];

	bool connect_out_flag;
};

#else

namespace Tritium {

class JackOutput : public NullDriver
{
public:
	JackOutput( audioProcessCallback processCallback ) : NullDriver( processCallback ) {}

};


#endif // JACK_SUPPORT

} // namespace Tritium

#endif // TRITIUM_JACKOUTPUT_HPP

