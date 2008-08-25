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

#include <hydrogen/IO/MidiInput.h>
#include <hydrogen/event_queue.h>
#include <hydrogen/Preferences.h>
#include <hydrogen/hydrogen.h>
#include <hydrogen/instrument.h>
#include <hydrogen/note.h>
#include <hydrogen/action.h>
#include <hydrogen/midiMap.h>

namespace H2Core
{

MidiInput::MidiInput( const QString class_name )
		: Object( class_name )
		, m_bActive( false )
{
	//INFOLOG( "INIT" );
	
}


MidiInput::~MidiInput()
{
	//INFOLOG( "DESTROY" );
}

void MidiInput::handleMidiMessage( const MidiMessage& msg )
{
	EventQueue::get_instance()->push_event( EVENT_MIDI_ACTIVITY, -1 );

//	infoLog( "[handleMidiMessage]" );
//	infoLog( "[handleMidiMessage] channel: " + to_string( msg.m_nChannel ) );
//	infoLog( "[handleMidiMessage] val1: " + to_string( msg.m_nData1 ) );
//	infoLog( "[handleMidiMessage] val2: " + to_string( msg.m_nData2 ) );

	switch ( msg.m_type ) {
	case MidiMessage::SYSEX:
		handleSysexMessage( msg );
		break;

	case MidiMessage::NOTE_ON:
		handleNoteOnMessage( msg );
		break;

	case MidiMessage::NOTE_OFF:
		handleNoteOffMessage( msg );
		break;

	case MidiMessage::POLYPHONIC_KEY_PRESSURE:
		ERRORLOG( "POLYPHONIC_KEY_PRESSURE event not handled yet" );
		break;

	case MidiMessage::CONTROL_CHANGE:
		INFOLOG( QString( "[handleMidiMessage] CONTROL_CHANGE Parameter: %1, Value: %2" ).arg( msg.m_nData1 ).arg( msg.m_nData2 ) );
		break;

	case MidiMessage::PROGRAM_CHANGE:
		INFOLOG( QString( "[handleMidiMessage] PROGRAM_CHANGE event, seting next pattern to %1" ).arg( msg.m_nData1 ) );
		Hydrogen::get_instance()->sequencer_setNextPattern(msg.m_nData1, false, false);
		break;

	case MidiMessage::CHANNEL_PRESSURE:
		ERRORLOG( "CHANNEL_PRESSURE event not handled yet" );
		break;

	case MidiMessage::PITCH_WHEEL:
		ERRORLOG( "PITCH_WHEEL event not handled yet" );
		break;

	case MidiMessage::SYSTEM_EXCLUSIVE:
		ERRORLOG( "SYSTEM_EXCLUSIVE event not handled yet" );
		break;

	case MidiMessage::START:
		INFOLOG( "START event" );
		if ( Hydrogen::get_instance()->getState() != STATE_PLAYING ) {
			Hydrogen::get_instance()->sequencer_play();
		}
		break;

	case MidiMessage::CONTINUE:
		ERRORLOG( "CONTINUE event not handled yet" );
		break;

	case MidiMessage::STOP:
		INFOLOG( "STOP event" );
		if ( Hydrogen::get_instance()->getState() == STATE_PLAYING ) {
			Hydrogen::get_instance()->sequencer_stop();
		}
		break;

	case MidiMessage::SONG_POS:
		ERRORLOG( "SONG_POS event not handled yet" );
		break;

	case MidiMessage::QUARTER_FRAME:
		WARNINGLOG( "QUARTER_FRAME event not handled yet" );
		break;

	case MidiMessage::UNKNOWN:
		ERRORLOG( "Unknown midi message" );
		break;

	default:
		ERRORLOG( QString( "unhandled midi message type: %1" ).arg( msg.m_type ) );
	}
}



void MidiInput::handleNoteOnMessage( const MidiMessage& msg )
{
	INFOLOG( "handleNoteOnMessage" );


	int nMidiChannelFilter = Preferences::getInstance()->m_nMidiChannelFilter;
	int nChannel = msg.m_nChannel;
	int nNote = msg.m_nData1;
	float fVelocity = msg.m_nData2 / 127.0;

	if ( fVelocity == 0 ) {
		handleNoteOffMessage( msg );
		return;
	}


	ActionManager * aH = ActionManager::getInstance();
	MidiMap * mM = MidiMap::getInstance();

	aH->handleAction( mM->getNoteAction( msg.m_nData1 ) );

	bool bIsChannelValid = true;
	if ( nMidiChannelFilter != -1 ) {
		bIsChannelValid = ( nChannel == nMidiChannelFilter );
	}

	Hydrogen *pEngine = Hydrogen::get_instance();

	bool bPatternSelect = false;

	if ( bIsChannelValid ) {
		if ( bPatternSelect ) {
			int patternNumber = nNote - 36;
			INFOLOG( QString( "next pattern = %1" ).arg( patternNumber ) );

			pEngine->sequencer_setNextPattern( patternNumber, false, false );
		} else {
			static const float fPan_L = 1.0f;
			static const float fPan_R = 1.0f;

			int nInstrument = nNote - 36;
			if ( nInstrument < 0 ) {
				nInstrument = 0;
			}
			if ( nInstrument > ( MAX_INSTRUMENTS -1 ) ) {
				nInstrument = MAX_INSTRUMENTS - 1;
			}

			pEngine->addRealtimeNote( nInstrument, fVelocity, fPan_L, fPan_R, 0.0, true, msg.m_use_frame, msg.m_frame );
		}
	}
}



void MidiInput::handleNoteOffMessage( const MidiMessage& msg )
{
	INFOLOG( "handleNoteOffMessage" );
	if ( Preferences::getInstance()->m_bMidiNoteOffIgnore ) {
		return;
	}

	Hydrogen *pEngine = Hydrogen::get_instance();
	Song *pSong = pEngine->getSong();

	int nNote = msg.m_nData1;
	int nInstrument = nNote - 36;
	if ( nInstrument < 0 ) {
		nInstrument = 0;
	}
	if ( nInstrument > ( MAX_INSTRUMENTS -1 ) ) {
		nInstrument = MAX_INSTRUMENTS - 1;
	}
	Instrument *pInstr = pSong->get_instrument_list()->get( nInstrument );
	unsigned nPosition = 0;
	const float fVelocity = 0.0f;
	const float fPan_L = 0.5f;
	const float fPan_R = 0.5f;
	const int nLenght = -1;
	const float fPitch = 0.0f;
	if (msg.m_use_frame) {
	    nPosition = pEngine->getRealtimeTickPosition(msg.m_frame);
	}
	Note *pNewNote = new Note( pInstr, nPosition, fVelocity, fPan_L, fPan_R, nLenght, fPitch );

	pEngine->midi_noteOff( pNewNote );
}



void MidiInput::handleSysexMessage( const MidiMessage& msg )
{

	/*
		General MMC message
		0	1	2	3	4	5
		F0	7F	id	6	cmd	247

		cmd:
		1	stop
		2	play
		3	Deferred play
		4	Fast Forward
		5	Rewind
		6	Record strobe (punch in)
		7	Record exit (punch out)
		9	Pause


		Goto MMC message
		0	1	2	3	4	5	6	7	8	9	10	11	12
		240	127	id	6	68	6	1	hr	mn	sc	fr	ff	247
	*/
	
	
	ActionManager * aH = ActionManager::getInstance();
	MidiMap * mM = MidiMap::getInstance();
	
	if ( msg.m_sysexData.size() == 6 ) {
		if (
		    ( msg.m_sysexData[0] == 0xF0 ) &&
		    ( msg.m_sysexData[1] == 127 ) &&
		    ( msg.m_sysexData[2] == 127 ) &&
		    ( msg.m_sysexData[3] == 6 ) ) {

			
			switch ( msg.m_sysexData[4] ) {

			case 1:	// STOP
			{ 
				aH->handleAction(mM->getMMCAction("MMC_STOP"));
				break;
			}

			case 2:	// PLAY
			{
				aH->handleAction(mM->getMMCAction("MMC_PLAY"));
				break;
			}

			case 3:	//DEFERRED PLAY
			{
				aH->handleAction(mM->getMMCAction("MMC_PLAY"));
				break;
			}

			case 4:	// FAST FWD
				aH->handleAction(mM->getMMCAction("MMC_FAST_FWD"));
				
				break;

			case 5:	// REWIND
				aH->handleAction(mM->getMMCAction("MMC_REWIND"));
				break;

			case 6:	// RECORD STROBE (PUNCH IN)
				aH->handleAction(mM->getMMCAction("MMC_RECORD_STROBE"));
				break;

			case 7:	// RECORD EXIT (PUNCH OUT)
				aH->handleAction(mM->getMMCAction("MMC_RECORD_EXIT"));
				break;

			case 9:	//PAUSE
				aH->handleAction(mM->getMMCAction("MMC_PAUSE"));
				break;

			default:
				WARNINGLOG( "Unknown MMC Command" );
//					midiDump( buf, nBytes );
			}
		}
	} else if ( msg.m_sysexData.size() == 13 ) {
		ERRORLOG( "MMC GOTO Message not implemented yet" );
//		midiDump( buf, nBytes );
		//int id = buf[2];
		int hr = msg.m_sysexData[7];
		int mn = msg.m_sysexData[8];
		int sc = msg.m_sysexData[9];
		int fr = msg.m_sysexData[10];
		int ff = msg.m_sysexData[11];
		char tmp[200];
		sprintf( tmp, "[handleSysexMessage] GOTO %d:%d:%d:%d:%d", hr, mn, sc, fr, ff );
		INFOLOG( tmp );

	} else {
		// sysex dump
		QString sDump = "";
		char tmpChar[64];
		for ( int i = 0; i < ( int )msg.m_sysexData.size(); ++i ) {
			sprintf( tmpChar, "%X ", ( int )msg.m_sysexData[ i ] );
			sDump += tmpChar;
		}
		WARNINGLOG( QString( "Unknown SysEx message: (%1) [%2]" ).arg( msg.m_sysexData.size() ).arg( sDump ) );
	}
}

int MidiInput::processAudio(uint32_t /*nframes*/)
{
    return 0;
}

int MidiInput::processNonAudio(uint32_t /*nframes*/)
{
    return 0;
}


};


