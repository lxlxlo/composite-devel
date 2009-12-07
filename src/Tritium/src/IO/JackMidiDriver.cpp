/*
 * Copyright(c) 2008,2009 by Gabriel M. Beddingfield <gabriel@teuton.org>
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
/* JackMidiDriver.cpp
 * Copyright(c) 2008 by Gabriel M. Beddingfield <gabriel@teuton.org>
 */

#include "JackMidiDriver.hpp"
#include "JackClient.hpp"
#include <Tritium/Logger.hpp>
#include <cassert>
#include <cstdlib> // free()
#include <Tritium/Engine.hpp>
#include <Tritium/Preferences.hpp> // For preferred auto-connection
#include <cerrno> // EEXIST for jack_connect()

#ifdef JACK_SUPPORT


/*********************************************************
 * Notes on JACK versions with respect to MIDI
 *
 * NOTE: COMPOSITE REQUIRES JACK 0.109.0 OR GREATER.
 *
 * First, jack/version.h defines the macro JACK_PROTOCOL_VERSION
 * to an integer... and this is *supposed* to be the reliable indicator
 * of our API.  However, this value is not reliable with respect
 * to JACK MIDI.  The following table describes the different
 * JACK versions and how they relate to the MIDI API.
 *
 * SVN  JACK Version PROTOCOL Milestone
 * ---- ------------ -------- -----------------------------
 *  847   0.99.37       15    Protocol set to 15
 *  ---   0.101.1       15    A public release
 *  945   0.102.0       15    Original MIDI API.
 *  968   0.102.10      16    Protocol set to 16
 *  ---   0.102.20      16    A public release
 *  998   0.102.27      16    MIDI API Changed:
 *                            jack_midi_port_get_info() replaced by
 *                              jack_midi_get_event_count()
 *                            jack_midi_reset_new_port() removed
 *  ---   0.103.0       16    A public release
 * 1030   0.103.0       19    Protocol set to 19
 * 1032   0.104.0       20    Protocol set to 20
 * 1035   0.105.0       20    MIDI API Changed:
 *                            The nframes paramter removed from these:
 *                              jack_midi_get_event_count()
 *                              jack_midi_event_get()
 *                              jack_midi_clear_buffer()
 *                              jack_midi_max_event_size()
 *                              jack_midi_event_reserve()
 *                              jack_midi_event_write()
 *                              jack_midi_get_lost_event_count()
 * 1046   0.107.0       21    Protocol set to 21
 *    ....
 *  ---   0.109.0       22    A public release
 *
 * It's also worth noting that the version numbers tend to change
 * with every commit.  However, the protocol version changes have
 * odd overlaps (with respect to MIDI).
 *
 ********************************************************
 */

using namespace std;
using namespace Tritium;

JackProcessCallback jackMidiFallbackProcess; // implemented in hydrogen.cpp

JackMidiDriver::JackMidiDriver(JackClient* parent)
	: MidiInput( "JackMidiDriver" ),
	  m_jack_client(parent),
	  m_port(0)
{
	INFOLOG( "CREATE" );
}

JackMidiDriver::~JackMidiDriver()
{
	INFOLOG( "DESTROY" );
	close();
}

void JackMidiDriver::open(void)
{
	JackClient& client = *m_jack_client;
	if (client.setNonAudioProcessCallback(jackMidiFallbackProcess)) {
		ERRORLOG("Could not set JACK process callback");
	}
	client.subscribe((void*)this);
	m_port = jack_port_register(client.ref(),
				    "midi_in",
				    JACK_DEFAULT_MIDI_TYPE,
				    JackPortIsInput,
				    0);
	if (!m_port) {
		ERRORLOG("Could not set JACK MIDI input port");
	}

	// Autoconnect port to an Output (readable) port
	QString OutPort = Hydrogen::get_instance()->get_preferences()->m_sMidiPortName;
	int err = jack_connect(client.ref(),
			       OutPort.toLatin1().constData(),
			       jack_port_name(m_port));
	if(err && (err != EEXIST)) {
		WARNINGLOG("Jack could not connect to port " + OutPort);
	}

}

void JackMidiDriver::close(void)
{
	if(m_port) {
		jack_client_t* client = m_jack_client->ref();
		if(client) {
			if (jack_port_unregister(client, m_port)) {
				ERRORLOG("JACK returned an error when unregistering port.");
			}
		m_jack_client->unsubscribe((void*)this);
		}
		m_port = 0;
	}
}


void translate_jack_midi_to_h2(Tritium::MidiMessage& msg,
			       const jack_midi_event_t& event,
			       bool use_frame)
{
	// For midi_commands, it's the programmer's responsibility
	// to ensure:
	//
	// a) The MSB is 1 (i.e. 1xxx xxxx)
	// b) the LSW is 0 (i.e. xxxx 0000)
	// c) That all 8 possible MSW's are present in the enum (0x8 thru 0xF)
	//
	// If not, casting ints to enums will fail:  either giving you
	// a runtime error or unexpected results.
	typedef enum {
		UNKNOWN = 0x00,
		NOTE_OFF = 0x80,
		NOTE_ON = 0x90,
		POLYPHONIC_KEY_PRESSURE = 0xA0,
		CONTROL_CHANGE = 0xB0,
		PROGRAM_CHANGE = 0xC0,
		CHANNEL_PRESSURE = 0xD0,
		PITCH_WHEEL = 0xE0,
		SYSTEM_EXCLUSIVE = 0xF0
	} midi_commands;
	// For sysex_messages, it's the programmer's responsibility
	// to ensure:
	//
	// a) the MSW is 0 (i.e. 0000 xxxx)
	// b) That all 16 possible LSW's are present in the enum (0x0 thru 0xF)
	//
	// If not, casting ints to enums will fail:  either giving you
	// a runtime error or unexpected results.
	typedef enum {
		SYSEX_START = 0x00,
		MTC_QUARTER_FRAME = 0x01,
		SONG_POS = 0x02,
		SONG_SELECT = 0x03,
		UNDEFINED_04 = 0x04,
		UNDEFINED_05 = 0x05,
		TUNE_REQ = 0x06,
		SYSEX_END = 0x07,
		CLOCK = 0x08,
		UNDEFINED_09 = 0x09,
		SONG_START = 0x0A,
		SONG_CONT = 0x0B,
		SONG_STOP = 0x0C,
		UNDEFINED_0D = 0x0D,
		ACTIVE_SENSING = 0x0E,
		SYSTEM_RESET = 0x0F
	} sysex_messages;
	

	midi_commands status;
	unsigned char tmp;
	msg.m_type = MidiMessage::UNKNOWN;
	msg.m_nData1 = -1;
	msg.m_nData2 = -1;
	msg.m_nChannel = -1;
	msg.m_sysexData.clear();

	if (event.size == 0)
		return;

	if (use_frame) {
		msg.m_use_frame = true;
		msg.m_frame = event.time;
	} else {
		msg.m_use_frame = false;
		msg.m_frame = 0;
	}
	tmp = event.buffer[0] & 0xF0;
	if (0x80 & tmp) {
		assert( (tmp & 0x8F) == 0x80 );
		status = (midi_commands)tmp;
	} else {
		status = UNKNOWN;
	}
	switch(status) {
	case UNKNOWN:
		msg = MidiMessage();
		break;
	case NOTE_ON:
		msg.m_type = MidiMessage::NOTE_ON;
		msg.m_nData1 = event.buffer[1];
		msg.m_nData2 = event.buffer[2];
		msg.m_nChannel = 0x0F & event.buffer[0];
		break;
	case NOTE_OFF:
		msg.m_type = MidiMessage::NOTE_OFF;
		msg.m_nData1 = event.buffer[1];
		msg.m_nData2 = event.buffer[2];
		msg.m_nChannel = 0x0F & event.buffer[0];
		break;
	case POLYPHONIC_KEY_PRESSURE:
		msg.m_type = MidiMessage::POLYPHONIC_KEY_PRESSURE;
		msg.m_nData1 = event.buffer[1];
		msg.m_nData2 = event.buffer[2];
		msg.m_nChannel = 0x0F & event.buffer[0];
		break;
	case CONTROL_CHANGE:
		msg.m_type = MidiMessage::CONTROL_CHANGE;
		msg.m_nData1 = event.buffer[1];
		msg.m_nData2 = event.buffer[2];
		msg.m_nChannel = 0x0F & event.buffer[0];
		break;
	case PROGRAM_CHANGE:
		msg.m_type = MidiMessage::PROGRAM_CHANGE;
		msg.m_nData1 = event.buffer[1];
		msg.m_nData2 = event.buffer[2];
		msg.m_nChannel = 0x0F & event.buffer[0];
		break;
	case CHANNEL_PRESSURE:
		msg.m_type = MidiMessage::CHANNEL_PRESSURE;
		msg.m_nData1 = event.buffer[1];
		msg.m_nData2 = -1;
		msg.m_nChannel = 0x0F & event.buffer[0];
		break;
	case PITCH_WHEEL:
		msg.m_type = MidiMessage::PITCH_WHEEL;
		msg.m_nData1 = event.buffer[1];
		msg.m_nData2 = event.buffer[2];
		msg.m_nChannel = 0x0F & event.buffer[0];
		break;
	case SYSTEM_EXCLUSIVE:
		assert( ((event.buffer[0] & 0x0F) & 0xF0) == 0 );
		switch (event.buffer[0] & 0x0F) {
		case SYSEX_START:
			msg.m_type = MidiMessage::SYSEX;
			msg.m_sysexData.assign(event.buffer+1, event.buffer+event.size);
			break;
		case MTC_QUARTER_FRAME:
			msg.m_type = MidiMessage::QUARTER_FRAME;
			msg.m_nData1 = event.buffer[1];
			break;
		case SONG_POS:
			msg.m_type = MidiMessage::SONG_POS;
			msg.m_nData1 = event.buffer[1];
			msg.m_nData2 = event.buffer[2];
			break;
		case SONG_START:
			msg.m_type = MidiMessage::START;
			break;
		case SONG_CONT:
			msg.m_type = MidiMessage::CONTINUE;
			break;
		case SONG_STOP:
			msg.m_type = MidiMessage::STOP;
			break;
			// Following not handled by Tritium::MidiMessage
		case SYSEX_END:
		case SONG_SELECT:
		case TUNE_REQ:
		case CLOCK:
		case ACTIVE_SENSING:
		case SYSTEM_RESET:
		case UNDEFINED_04:
		case UNDEFINED_05:
		case UNDEFINED_09:
		case UNDEFINED_0D:
			msg.m_type = MidiMessage::UNKNOWN;
			break;
		default:
			assert(false);  // Should not reach this line
		}
		break;
	default:
		assert(false); // Should not reach this line
	}
}

int JackMidiDriver::processAudio(jack_nframes_t nframes)
{
	return process(nframes, true);
}

int JackMidiDriver::processNonAudio(jack_nframes_t nframes)
{
	return process(nframes, false);
}

// This function must be realtime safe.  It will be called from
// the JACK process callback.
int JackMidiDriver::process(jack_nframes_t nframes, bool use_frame)
{
	if (!m_port) return 0;

	jack_nframes_t event_ct, event_pos;
	jack_midi_event_t jack_event;
	Tritium::MidiMessage msg;

	void* port_buf = jack_port_get_buffer(m_port, nframes);
	event_ct = jack_midi_get_event_count(port_buf);

	for ( event_pos=0 ; event_pos<event_ct ; ++event_pos ) {
		if ( jack_midi_event_get(&jack_event, port_buf, event_pos) ) {
			break;
		}
		translate_jack_midi_to_h2(msg, jack_event, use_frame);
		if (msg.m_type != MidiMessage::UNKNOWN) {
			handleMidiMessage(msg);
		}
	}
	return 0;
}

std::vector<QString> JackMidiDriver::getOutputPortList(void)
{
	return m_jack_client->getMidiOutputPortList();
}

#endif // JACK_SUPPORT
