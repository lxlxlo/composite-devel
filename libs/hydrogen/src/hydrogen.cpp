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

#ifdef WIN32
#include "timeHelper.h"
#include "timersub.h"
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#include <pthread.h>
#include <cassert>
#include <cstdio>
#include <deque>
#include <iostream>
#include <ctime>
#include <cmath>

#include <hydrogen/LocalFileMng.h>
#include <hydrogen/event_queue.h>
#include <hydrogen/adsr.h>
#include <hydrogen/SoundLibrary.h>
#include <hydrogen/h2_exception.h>
#include <hydrogen/audio_engine.h>
#include <hydrogen/instrument.h>
#include <hydrogen/sample.h>
#include <hydrogen/hydrogen.h>
#include <hydrogen/Pattern.h>
#include <hydrogen/note.h>
#include <hydrogen/fx/LadspaFX.h>
#include <hydrogen/fx/Effects.h>
#include <hydrogen/IO/AudioOutput.h>
#include <hydrogen/IO/JackOutput.h>
#include <hydrogen/IO/NullDriver.h>
#include <hydrogen/IO/MidiInput.h>
#include <hydrogen/IO/CoreMidiDriver.h>
#include <hydrogen/IO/TransportInfo.h>
#include <hydrogen/Preferences.h>
#include <hydrogen/data_path.h>
#include <hydrogen/sampler/Sampler.h>

#include "IO/OssDriver.h"
#include "IO/FakeDriver.h"
#include "IO/AlsaAudioDriver.h"
#include "IO/PortAudioDriver.h"
#include "IO/DiskWriterDriver.h"
#include "IO/AlsaMidiDriver.h"
#include "IO/PortMidiDriver.h"
#include "IO/CoreAudioDriver.h"


namespace H2Core
{

// GLOBALS

// info
float m_fMasterPeak_L = 0.0f;		///< Master peak (left channel)
float m_fMasterPeak_R = 0.0f;		///< Master peak (right channel)
float m_fProcessTime = 0.0f;		///< time used in process function
float m_fMaxProcessTime = 0.0f;		///< max ms usable in process with no xrun
//~ info


AudioOutput *m_pAudioDriver = NULL;	///< Audio output
MidiInput *m_pMidiDriver = NULL;	///< MIDI input

std::deque<Note*> m_songNoteQueue;	///< Song Note FIFO
std::deque<Note*> m_midiNoteQueue;	///< Midi Note FIFO


Song *m_pSong;				///< Current song
PatternList* m_pNextPatterns;		///< Next pattern (used only in Pattern mode)
bool m_bAppendNextPattern;		///< Add the next pattern to the list instead of replace.
bool m_bDeleteNextPattern;		///< Delete the next pattern from the list.


PatternList* m_pPlayingPatterns;
int m_nSongPos;				///< Is the position inside the song

int m_nSelectedPatternNumber;
int m_nSelectedInstrumentNumber;

Instrument *m_pMetronomeInstrument = NULL;	///< Metronome instrument


// Buffers used in the process function
unsigned m_nBufferSize = 0;
float *m_pMainBuffer_L = NULL;
float *m_pMainBuffer_R = NULL;
bool m_bUseDefaultOuts = false;


Hydrogen* hydrogenInstance = NULL;		///< Hydrogen class instance (used for log)


int  m_audioEngineState = STATE_UNINITIALIZED;	///< Audio engine state



#ifdef LADSPA_SUPPORT
float m_fFXPeak_L[MAX_FX];
float m_fFXPeak_R[MAX_FX];
#endif


int m_nPatternStartTick = -1;
int m_nPatternTickPosition = 0;

// used in findPatternInTick
int m_nSongSizeInTicks = 0;

struct timeval m_currentTickTime;

unsigned long m_nRealtimeFrames = 0;




// PROTOTYPES
void	audioEngine_init();
void	audioEngine_destroy();
int	audioEngine_start( bool bLockEngine = false, unsigned nTotalFrames = 0 );
void	audioEngine_stop( bool bLockEngine = false );
void	audioEngine_setSong( Song *newSong );
void	audioEngine_removeSong();
void	audioEngine_noteOn( Note *note );
void	audioEngine_noteOff( Note *note );
int	audioEngine_process( uint32_t nframes, void *arg );
inline void audioEngine_clearNoteQueue();
inline void audioEngine_process_checkBPMChanged();
inline void audioEngine_process_playNotes( unsigned long nframes );
inline void audioEngine_process_transport();

inline unsigned audioEngine_renderNote( Note* pNote, const unsigned& nBufferSize );
inline int audioEngine_updateNoteQueue( unsigned nFrames );

inline int findPatternInTick( int tick, bool loopMode, int *patternStartTick );

void audioEngine_seek( long long nFrames, bool bLoopMode = false );

void audioEngine_restartAudioDrivers();
void audioEngine_startAudioDrivers();
void audioEngine_stopAudioDrivers();

/*
inline unsigned long currentTime() {
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec * 1000 + now.tv_usec / 1000;
}
*/


inline timeval currentTime2()
{
	struct timeval now;
	gettimeofday( &now, NULL );
	return now;
}



inline int randomValue( int max )
{
	return rand() % max;
}


inline float getGaussian( float z )
{
	// gaussian distribution -- dimss
	float x1, x2, w;
	do {
		x1 = 2.0 * ( ( ( float ) rand() ) / RAND_MAX ) - 1.0;
		x2 = 2.0 * ( ( ( float ) rand() ) / RAND_MAX ) - 1.0;
		w = x1 * x1 + x2 * x2;
	} while ( w >= 1.0 );

	w = sqrtf( ( -2.0 * logf( w ) ) / w );
	return x1 * w * z + 0.5; // tunable
}



void audioEngine_raiseError( unsigned nErrorCode )
{
	EventQueue::get_instance()->push_event( EVENT_ERROR, nErrorCode );
}



void updateTickSize()
{
//	hydrogenInstance->infoLog("UpdateTickSize");
	float sampleRate = ( float )m_pAudioDriver->getSampleRate();
	m_pAudioDriver->m_transport.m_nTickSize = ( sampleRate * 60.0 /  m_pSong->__bpm / m_pSong->__resolution );
}



void audioEngine_init()
{
	_INFOLOG( "*** Hydrogen audio engine init ***" );

	// check current state
	if ( m_audioEngineState != STATE_UNINITIALIZED ) {
		_ERRORLOG( "Error the audio engine is not in UNINITIALIZED state" );
		AudioEngine::get_instance()->unlock();
		return;
	}

	m_pSong = NULL;
	m_pPlayingPatterns = new PatternList();
	m_pNextPatterns = new PatternList();
	m_nSongPos = -1;
	m_nSelectedPatternNumber = 0;
	m_nSelectedInstrumentNumber = 0;
	m_nPatternTickPosition = 0;
	m_pMetronomeInstrument = NULL;
	m_pAudioDriver = NULL;

	m_pMainBuffer_L = NULL;
	m_pMainBuffer_R = NULL;

// 	for (unsigned i=0; i < MAX_INSTRUMENTS; ++i) {
// 		m_pTrackBuffers_L[i] = NULL;
// 		m_pTrackBuffers_R[i] = NULL;
// 	}

	Preferences *preferences = Preferences::getInstance();
	m_bUseDefaultOuts = preferences->m_bJackConnectDefaults;


	srand( time( NULL ) );

	// Create metronome instrument
	std::string sMetronomeFilename = std::string( DataPath::get_data_path() ) + "/click.wav";
	m_pMetronomeInstrument = new Instrument( sMetronomeFilename, "metronome", new ADSR() );
	m_pMetronomeInstrument->set_layer( new InstrumentLayer( Sample::load( sMetronomeFilename ) ), 0 );

	// Change the current audio engine state
	m_audioEngineState = STATE_INITIALIZED;

	EventQueue::get_instance()->push_event( EVENT_STATE, STATE_INITIALIZED );
}



void audioEngine_destroy()
{
	AudioEngine::get_instance()->lock( "audioEngine_destroy" );
	_INFOLOG( "*** Hydrogen audio engine shutdown ***" );

	// check current state
	if ( m_audioEngineState != STATE_INITIALIZED ) {
		_ERRORLOG( "Error the audio engine is not in INITIALIZED state" );
		return;
	}
	AudioEngine::get_instance()->get_sampler()->stop_playing_notes();

	// delete all copied notes in the song notes queue
	for ( unsigned i = 0; i < m_songNoteQueue.size(); ++i ) {
		Note *note = m_songNoteQueue[i];
		delete note;
		note = NULL;
	}
	m_songNoteQueue.clear();

	// delete all copied notes in the midi notes queue
	for ( unsigned i = 0; i < m_midiNoteQueue.size(); ++i ) {
		Note *note = m_midiNoteQueue[i];
		delete note;
		note = NULL;
	}
	m_midiNoteQueue.clear();

	// change the current audio engine state
	m_audioEngineState = STATE_UNINITIALIZED;

	EventQueue::get_instance()->push_event( EVENT_STATE, STATE_UNINITIALIZED );

	delete m_pPlayingPatterns;
	m_pPlayingPatterns = NULL;

	delete m_pNextPatterns;
	m_pNextPatterns = NULL;

	delete m_pMetronomeInstrument;
	m_pMetronomeInstrument = NULL;

	AudioEngine::get_instance()->unlock();
}





/// Start playing
/// return 0 = OK
/// return -1 = NULL Audio Driver
/// return -2 = Driver connect() error
int audioEngine_start( bool bLockEngine, unsigned nTotalFrames )
{
	if ( bLockEngine ) {
		AudioEngine::get_instance()->lock( "audioEngine_start" );
	}

	_INFOLOG( "[audioEngine_start]" );

	// check current state
	if ( m_audioEngineState != STATE_READY ) {
		_ERRORLOG( "Error the audio engine is not in READY state" );
		if ( bLockEngine ) {
			AudioEngine::get_instance()->unlock();
		}
		return 0;	// FIXME!!
	}


	//Preferences *preferencesMng = Preferences::getInstance();
	m_fMasterPeak_L = 0.0f;
	m_fMasterPeak_R = 0.0f;
	m_pAudioDriver->m_transport.m_nFrames = nTotalFrames;	// reset total frames
	m_nSongPos = -1;
	m_nPatternStartTick = -1;
	m_nPatternTickPosition = 0;

	// prepare the tickSize for this song
	updateTickSize();

	// change the current audio engine state
	m_audioEngineState = STATE_PLAYING;
	EventQueue::get_instance()->push_event( EVENT_STATE, STATE_PLAYING );

	if ( bLockEngine ) {
		AudioEngine::get_instance()->unlock();
	}
	return 0; // per ora restituisco sempre OK
}



/// Stop the audio engine
void audioEngine_stop( bool bLockEngine )
{
	if ( bLockEngine ) {
		AudioEngine::get_instance()->lock( "audioEngine_stop" );
	}
	_INFOLOG( "[audioEngine_stop]" );

	// check current state
	if ( m_audioEngineState != STATE_PLAYING ) {
		_ERRORLOG( "Error the audio engine is not in PLAYING state" );
		if ( bLockEngine ) {
			AudioEngine::get_instance()->unlock();
		}
		return;
	}

	// change the current audio engine state
	m_audioEngineState = STATE_READY;
	EventQueue::get_instance()->push_event( EVENT_STATE, STATE_READY );

	m_fMasterPeak_L = 0.0f;
	m_fMasterPeak_R = 0.0f;
//	m_nPatternTickPosition = 0;
	m_nPatternStartTick = -1;

	// delete all copied notes in the song notes queue
	for ( unsigned i = 0; i < m_songNoteQueue.size(); ++i ) {
		Note *note = m_songNoteQueue[i];
		delete note;
	}
	m_songNoteQueue.clear();

	/*	// delete all copied notes in the playing notes queue
		for (unsigned i = 0; i < m_playingNotesQueue.size(); ++i) {
			Note *note = m_playingNotesQueue[i];
			delete note;
		}
		m_playingNotesQueue.clear();
	*/

	// delete all copied notes in the midi notes queue
	for ( unsigned i = 0; i < m_midiNoteQueue.size(); ++i ) {
		Note *note = m_midiNoteQueue[i];
		delete note;
	}
	m_midiNoteQueue.clear();


	if ( bLockEngine ) {
		AudioEngine::get_instance()->unlock();
	}
}

//
///  Update Tick size and frame position in the audio driver from Song->__bpm
//
inline void audioEngine_process_checkBPMChanged()
{

	if ( ( m_audioEngineState == STATE_READY ) || ( m_audioEngineState == STATE_PLAYING ) ) {

		float fNewTickSize = m_pAudioDriver->getSampleRate() * 60.0 /  m_pSong->__bpm / m_pSong->__resolution;

		if ( fNewTickSize != m_pAudioDriver->m_transport.m_nTickSize ) {
			// cerco di convertire ...
			float fTickNumber = ( float )m_pAudioDriver->m_transport.m_nFrames / ( float )m_pAudioDriver->m_transport.m_nTickSize;

			m_pAudioDriver->m_transport.m_nTickSize = fNewTickSize;

			if ( m_pAudioDriver->m_transport.m_nTickSize == 0 ) {
				return;
			}

			_WARNINGLOG( "Tempo change: Recomputing ticksize and frame position" );
			long long nNewFrames = ( long long )( fTickNumber * fNewTickSize );
			// update frame position
			m_pAudioDriver->m_transport.m_nFrames = nNewFrames;
#ifdef JACK_SUPPORT
			if ( "JackOutput" == m_pAudioDriver->get_class_name() && m_audioEngineState == STATE_PLAYING ) {
				static_cast< JackOutput* >( m_pAudioDriver )->calculateFrameOffset();
			}
#endif

			/*

						// delete all copied notes in the song notes queue
						for (unsigned i = 0; i < m_songNoteQueue.size(); ++i) {
							delete m_songNoteQueue[i];
						}
						m_songNoteQueue.clear();

						// send a note-off event to all notes present in the playing note queue
						for ( int i = 0; i < m_playingNotesQueue.size(); ++i ) {
							Note *pNote = m_playingNotesQueue[ i ];
							pNote->m_pADSR->release();
						}

						// delete all copied notes in the midi notes queue
						for (unsigned i = 0; i < m_midiNoteQueue.size(); ++i) {
							delete m_midiNoteQueue[i];
						}
						m_midiNoteQueue.clear();
			*/
		}
	}
}



inline void audioEngine_process_playNotes( unsigned long nframes )
{
	unsigned int framepos;

	if (  m_audioEngineState == STATE_PLAYING ) {
		framepos = m_pAudioDriver->m_transport.m_nFrames;
	} else {
		// use this to support realtime events when not playing
		framepos = m_nRealtimeFrames;
	}

	// leggo da m_songNoteQueue
	while ( m_songNoteQueue.size() > 0 ) {
		Note *pNote = m_songNoteQueue[0];

		// verifico se la nota rientra in questo ciclo
		unsigned noteStartInFrames = ( unsigned )( pNote->get_position() * m_pAudioDriver->m_transport.m_nTickSize );

		// m_nTotalFrames <= NotePos < m_nTotalFrames + bufferSize
		bool isNoteStart = ( ( noteStartInFrames >= framepos ) && ( noteStartInFrames < ( framepos + nframes ) ) );
		bool isOldNote = noteStartInFrames < framepos;
		if ( isNoteStart || isOldNote ) {

			// Humanize - Velocity parameter
			if ( m_pSong->get_humanize_velocity_value() != 0 ) {
				float random = m_pSong->get_humanize_velocity_value() * getGaussian( 0.2 );
				pNote->set_velocity( pNote->get_velocity() + ( random - ( m_pSong->get_humanize_velocity_value() / 2.0 ) ) );
				if ( pNote->get_velocity() > 1.0 ) {
					pNote->set_velocity( 1.0 );
				} else if ( pNote->get_velocity() < 0.0 ) {
					pNote->set_velocity( 0.0 );
				}
			}

			// Random Pitch ;)
			const float fMaxPitchDeviation = 2.0;
			pNote->set_pitch( pNote->get_pitch() + ( fMaxPitchDeviation * getGaussian( 0.2 ) - fMaxPitchDeviation / 2.0 ) * pNote->get_instrument()->get_random_pitch_factor() );

			AudioEngine::get_instance()->get_sampler()->note_on( pNote );	// aggiungo la nota alla lista di note da eseguire
			m_songNoteQueue.pop_front();			// rimuovo la nota dalla lista di note

			// raise noteOn event
			int nInstrument = m_pSong->get_instrument_list()->get_pos( pNote->get_instrument() );
			EventQueue::get_instance()->push_event( EVENT_NOTEON, nInstrument );
			continue;
		} else {
			// la nota non andra' in esecuzione
			break;
		}
	}

}


void audioEngine_seek( long long nFrames, bool bLoopMode )
{
	if ( m_pAudioDriver->m_transport.m_nFrames == nFrames ) {
		return;
	}

	if ( nFrames < 0 ) {
		_ERRORLOG( "nFrames < 0" );
	}

	char tmp[200];
	sprintf( tmp, "seek in %lld (old pos = %d)", nFrames, ( int )m_pAudioDriver->m_transport.m_nFrames );
	_INFOLOG( tmp );

	m_pAudioDriver->m_transport.m_nFrames = nFrames;

	int tickNumber_start = ( unsigned )( m_pAudioDriver->m_transport.m_nFrames / m_pAudioDriver->m_transport.m_nTickSize );
//	sprintf(tmp, "[audioEngine_seek()] tickNumber_start = %d", tickNumber_start);
//	hydrogenInstance->infoLog(tmp);

	bool loop = m_pSong->is_loop_enabled();

	if ( bLoopMode ) {
		loop = true;
	}

	m_nSongPos = findPatternInTick( tickNumber_start, loop, &m_nPatternStartTick );
//	sprintf(tmp, "[audioEngine_seek()] m_nSongPos = %d", m_nSongPos);
//	hydrogenInstance->infoLog(tmp);

	audioEngine_clearNoteQueue();
}



inline void audioEngine_process_transport()
{
	if ( ( m_audioEngineState == STATE_READY ) || ( m_audioEngineState == STATE_PLAYING ) ) {
		m_pAudioDriver->updateTransportInfo();
		unsigned long nNewFrames = m_pAudioDriver->m_transport.m_nFrames;

		// ??? audioEngine_seek returns IMMEDIATELY when nNewFrames == m_pAudioDriver->m_transport.m_nFrames ???
// 		audioEngine_seek( nNewFrames, true );

		switch ( m_pAudioDriver->m_transport.m_status ) {
		case TransportInfo::ROLLING:

//				hydrogenInstance->infoLog( "[audioEngine_process_transport] ROLLING - frames: " + to_string(m_pAudioDriver->m_transport.m_nFrames) );
			if ( m_audioEngineState == STATE_READY ) {
				//hydrogenInstance->infoLog( "[audioEngine_process_transport] first start frames is " + to_string(m_pAudioDriver->m_transport.m_nFrames) );
				audioEngine_start( false, nNewFrames );	// no engine lock
			}

			if ( m_pSong->__bpm != m_pAudioDriver->m_transport.m_nBPM ) {
				_INFOLOG( "song bpm: (" + to_string( m_pSong->__bpm ) + ") gets transport bpm: (" + to_string( m_pAudioDriver->m_transport.m_nBPM ) + ")" );

				m_pSong->__bpm = m_pAudioDriver->m_transport.m_nBPM;
			}

			m_nRealtimeFrames = m_pAudioDriver->m_transport.m_nFrames;
			break;


		case TransportInfo::STOPPED:

//				hydrogenInstance->infoLog( "[audioEngine_process_transport] STOPPED - frames: " + to_string(m_pAudioDriver->m_transport.m_nFrames) );
			if ( m_audioEngineState == STATE_PLAYING ) {
				audioEngine_stop( false );	// no engine lock
			}

			if ( m_pSong->__bpm != m_pAudioDriver->m_transport.m_nBPM ) {
				m_pSong->__bpm = m_pAudioDriver->m_transport.m_nBPM;
			}

			// go ahead and increment the realtimeframes by buffersize
			// to support our realtime keyboard and midi event timing
			m_nRealtimeFrames += m_nBufferSize;
			break;
		}
	}
}



void audioEngine_clearNoteQueue()
{
	//_INFOLOG( "clear notes...");

	for ( unsigned i = 0; i < m_songNoteQueue.size(); ++i ) {	// delete all copied notes in the song notes queue
		delete m_songNoteQueue[i];
	}
	m_songNoteQueue.clear();

	/*	for (unsigned i = 0; i < m_playingNotesQueue.size(); ++i) {	// delete all copied notes in the playing notes queue
			delete m_playingNotesQueue[i];
		}
		m_playingNotesQueue.clear();
	*/
	AudioEngine::get_instance()->get_sampler()->stop_playing_notes();

	for ( unsigned i = 0; i < m_midiNoteQueue.size(); ++i ) {	// delete all copied notes in the midi notes queue
		delete m_midiNoteQueue[i];
	}
	m_midiNoteQueue.clear();

}



/// Clear all audio buffers
inline void audioEngine_process_clearAudioBuffers( uint32_t nFrames )
{
	if ( m_pMainBuffer_L ) {
		memset( m_pMainBuffer_L, 0, nFrames * sizeof( float ) );	// clear main out Left
	}
	if ( m_pMainBuffer_R ) {
		memset( m_pMainBuffer_R, 0, nFrames * sizeof( float ) );	// clear main out Right
	}

	if ( ( m_audioEngineState == STATE_READY ) || ( m_audioEngineState == STATE_PLAYING ) ) {
#ifdef LADSPA_SUPPORT
		Effects* pEffects = Effects::getInstance();
		for ( unsigned i = 0; i < MAX_FX; ++i ) {	// clear FX buffers
			LadspaFX* pFX = pEffects->getLadspaFX( i );
			if ( pFX ) {
				assert( pFX->m_pBuffer_L );
				assert( pFX->m_pBuffer_R );
				memset( pFX->m_pBuffer_L, 0, nFrames * sizeof( float ) );
				memset( pFX->m_pBuffer_R, 0, nFrames * sizeof( float ) );
			}
		}
#endif
	}
}

/// Main audio processing function. Called by audio drivers.
int audioEngine_process( uint32_t nframes, void *arg )
{
	UNUSED( arg );

	if ( AudioEngine::get_instance()->try_lock( "audioEngine_process" ) == false ) {
		return 0;
	}

	timeval startTimeval = currentTime2();

	if ( m_nBufferSize != nframes ) {
		_INFOLOG( "Buffer size changed. Old size = " + to_string( m_nBufferSize ) +", new size = " + to_string( nframes ) );
		m_nBufferSize = nframes;
	}

	audioEngine_process_transport();		// m_pAudioDriver->bpm updates Song->__bpm. (!!(Calls audioEngine_seek))
	audioEngine_process_clearAudioBuffers( nframes );
	audioEngine_process_checkBPMChanged();		// m_pSong->__bpm decides tick size

	bool sendPatternChange = false;
	// always update note queue.. could come from pattern or realtime input (midi, keyboard)
	int res2 = audioEngine_updateNoteQueue( nframes );	// update the notes queue
	if ( res2 == -1 ) {	// end of song
		_INFOLOG( "End of song received, calling engine_stop()" );
		AudioEngine::get_instance()->unlock();
		m_pAudioDriver->stop();
		m_pAudioDriver->locate( 0 );	// locate 0, reposition from start of the song

		if ( ( m_pAudioDriver->get_class_name() == "DiskWriterDriver" ) || ( m_pAudioDriver->get_class_name() == "FakeDriver" ) ) {
			_INFOLOG( "End of song." );
			return 1;	// kill the audio AudioDriver thread
		}

		return 0;
	} else if ( res2 == 2 ) {	// send pattern change
		sendPatternChange = true;
	}


	// play all notes
	audioEngine_process_playNotes( nframes );


	timeval renderTime_start = currentTime2();

	// SAMPLER
	AudioEngine::get_instance()->get_sampler()->process( nframes, m_pSong );
	float* out_L = AudioEngine::get_instance()->get_sampler()->__main_out_L;
	float* out_R = AudioEngine::get_instance()->get_sampler()->__main_out_R;
	for ( unsigned i = 0; i < nframes; ++i ) {
		m_pMainBuffer_L[ i ] += out_L[ i ];
		m_pMainBuffer_R[ i ] += out_R[ i ];
	}

	// SYNTH
	AudioEngine::get_instance()->get_synth()->process( nframes );
	out_L = AudioEngine::get_instance()->get_synth()->m_pOut_L;
	out_R = AudioEngine::get_instance()->get_synth()->m_pOut_R;
	for ( unsigned i = 0; i < nframes; ++i ) {
		m_pMainBuffer_L[ i ] += out_L[ i ];
		m_pMainBuffer_R[ i ] += out_R[ i ];
	}


	timeval renderTime_end = currentTime2();



	timeval ladspaTime_start = renderTime_end;
#ifdef LADSPA_SUPPORT
	// Process LADSPA FX
	if ( ( m_audioEngineState == STATE_READY ) || ( m_audioEngineState == STATE_PLAYING ) ) {
		for ( unsigned nFX = 0; nFX < MAX_FX; ++nFX ) {
			LadspaFX *pFX = Effects::getInstance()->getLadspaFX( nFX );
			if ( ( pFX ) && ( pFX->isEnabled() ) ) {
				pFX->processFX( nframes );
				float *buf_L = NULL;
				float *buf_R = NULL;
				if ( pFX->getPluginType() == LadspaFX::STEREO_FX ) {	// STEREO FX
					buf_L = pFX->m_pBuffer_L;
					buf_R = pFX->m_pBuffer_R;
				} else { // MONO FX
					buf_L = pFX->m_pBuffer_L;
					buf_R = buf_L;
				}
				for ( unsigned i = 0; i < nframes; ++i ) {
					m_pMainBuffer_L[ i ] += buf_L[ i ];
					m_pMainBuffer_R[ i ] += buf_R[ i ];
					if ( buf_L[ i ] > m_fFXPeak_L[nFX] )	m_fFXPeak_L[nFX] = buf_L[ i ];
					if ( buf_R[ i ] > m_fFXPeak_R[nFX] )	m_fFXPeak_R[nFX] = buf_R[ i ];
				}
			}
		}
	}
#endif
	timeval ladspaTime_end = currentTime2();

	// update master peaks
	float val_L;
	float val_R;
	if ( m_audioEngineState == STATE_PLAYING || m_audioEngineState == STATE_READY ) {
		for ( unsigned i = 0; i < nframes; ++i ) {
			val_L = m_pMainBuffer_L[i];
			val_R = m_pMainBuffer_R[i];
			if ( val_L > m_fMasterPeak_L ) {
				m_fMasterPeak_L = val_L;
			}
			if ( val_R > m_fMasterPeak_R ) {
				m_fMasterPeak_R = val_R;
			}
		}
	}

	// update total frames number
	if ( m_audioEngineState == STATE_PLAYING ) {
		m_pAudioDriver->m_transport.m_nFrames += nframes;
	}

//	float fRenderTime = (renderTime_end.tv_sec - renderTime_start.tv_sec) * 1000.0 + (renderTime_end.tv_usec - renderTime_start.tv_usec) / 1000.0;
	float fLadspaTime = ( ladspaTime_end.tv_sec - ladspaTime_start.tv_sec ) * 1000.0 + ( ladspaTime_end.tv_usec - ladspaTime_start.tv_usec ) / 1000.0;


	timeval finishTimeval = currentTime2();
	m_fProcessTime = ( finishTimeval.tv_sec - startTimeval.tv_sec ) * 1000.0 + ( finishTimeval.tv_usec - startTimeval.tv_usec ) / 1000.0;

	float sampleRate = ( float )m_pAudioDriver->getSampleRate();
	m_fMaxProcessTime = 1000.0 / ( sampleRate / nframes );


	//DEBUG
	if ( m_fProcessTime > m_fMaxProcessTime ) {
		_WARNINGLOG( "" );
		_WARNINGLOG( "----XRUN----" );
		_WARNINGLOG( "XRUN of " + to_string( ( m_fProcessTime - m_fMaxProcessTime ) ) + std::string( " msec (" ) + to_string( m_fProcessTime ) + std::string( " > " ) + to_string( m_fMaxProcessTime ) + std::string( ")" ) );
		_WARNINGLOG( "Ladspa process time = " + to_string( fLadspaTime ) );
		_WARNINGLOG( "------------" );
		_WARNINGLOG( "" );
		// raise xRun event
		EventQueue::get_instance()->push_event( EVENT_XRUN, -1 );
	}

	AudioEngine::get_instance()->unlock();

	if ( sendPatternChange ) {
		EventQueue::get_instance()->push_event( EVENT_PATTERN_CHANGED, -1 );
	}

	return 0;
}





void audioEngine_setupLadspaFX( unsigned nBufferSize )
{
	//_INFOLOG( "buffersize=" + to_string(nBufferSize) );

	if ( m_pSong == NULL ) {
		//_INFOLOG( "m_pSong=NULL" );
		return;
	}
	if ( nBufferSize == 0 ) {
		_ERRORLOG( "nBufferSize=0" );
		return;
	}

#ifdef LADSPA_SUPPORT
	for ( unsigned nFX = 0; nFX < MAX_FX; ++nFX ) {
		LadspaFX *pFX = Effects::getInstance()->getLadspaFX( nFX );
		if ( pFX == NULL ) {
			return;
		}

		pFX->deactivate();

//		delete[] pFX->m_pBuffer_L;
//		pFX->m_pBuffer_L = NULL;
//		delete[] pFX->m_pBuffer_R;
//		pFX->m_pBuffer_R = NULL;
//		if ( nBufferSize != 0 ) {
		//pFX->m_nBufferSize = nBufferSize;
		//pFX->m_pBuffer_L = new float[ nBufferSize ];
		//pFX->m_pBuffer_R = new float[ nBufferSize ];
//		}

		Effects::getInstance()->getLadspaFX( nFX )->connectAudioPorts(
		    pFX->m_pBuffer_L,
		    pFX->m_pBuffer_R,
		    pFX->m_pBuffer_L,
		    pFX->m_pBuffer_R
		);
		pFX->activate();
	}
#endif
}



void audioEngine_renameJackPorts()
{
#ifdef JACK_SUPPORT
	// renames jack ports
	if ( m_pSong == NULL ) {
		return;
	}
	if ( m_pAudioDriver->get_class_name() == "JackOutput" ) {
		static_cast< JackOutput* >( m_pAudioDriver )->makeTrackOutputs( m_pSong );
	}

	AudioEngine::get_instance()->get_sampler()->makeTrackOutputQueues();

#endif
}



void audioEngine_setSong( Song *newSong )
{
	_WARNINGLOG( "set song: " + newSong->__name );

	AudioEngine::get_instance()->lock( "audioEngine_setSong" );

	if ( m_audioEngineState == STATE_PLAYING ) {
		m_pAudioDriver->stop();
		audioEngine_stop( false );
	}

	// check current state
	if ( m_audioEngineState != STATE_PREPARED ) {
		_ERRORLOG( "Error the audio engine is not in PREPARED state" );
	}

	m_pPlayingPatterns->clear();
	m_pNextPatterns->clear();

	EventQueue::get_instance()->push_event( EVENT_SELECTED_PATTERN_CHANGED, -1 );
	EventQueue::get_instance()->push_event( EVENT_PATTERN_CHANGED, -1 );
	EventQueue::get_instance()->push_event( EVENT_SELECTED_INSTRUMENT_CHANGED, -1 );

	//sleep( 1 );

	audioEngine_clearNoteQueue();

	assert( m_pSong == NULL );
	m_pSong = newSong;

	// setup LADSPA FX
	audioEngine_setupLadspaFX( m_pAudioDriver->getBufferSize() );

	// update ticksize
	audioEngine_process_checkBPMChanged();

	// find the first pattern and set as current
	if ( m_pSong->get_pattern_list()->get_size() > 0 ) {
		m_pPlayingPatterns->add( m_pSong->get_pattern_list()->get( 0 ) );
	}


	audioEngine_renameJackPorts();

	m_pAudioDriver->setBpm( m_pSong->__bpm );

	// change the current audio engine state
	m_audioEngineState = STATE_READY;

	m_pAudioDriver->locate( 0 );

	AudioEngine::get_instance()->unlock();

	EventQueue::get_instance()->push_event( EVENT_STATE, STATE_READY );
}



void audioEngine_removeSong()
{
	AudioEngine::get_instance()->lock( "audioEngine_removeSong" );

	if ( m_audioEngineState == STATE_PLAYING ) {
		m_pAudioDriver->stop();
		audioEngine_stop( false );
	}

	// check current state
	if ( m_audioEngineState != STATE_READY ) {
		_ERRORLOG( "Error the audio engine is not in READY state" );
		AudioEngine::get_instance()->unlock();
		return;
	}

	m_pSong = NULL;
	m_pPlayingPatterns->clear();
	m_pNextPatterns->clear();

	audioEngine_clearNoteQueue();

	// change the current audio engine state
	m_audioEngineState = STATE_PREPARED;
	AudioEngine::get_instance()->unlock();

	EventQueue::get_instance()->push_event( EVENT_STATE, STATE_PREPARED );
}



// return -1 = end of song
// return 2 = send pattern changed event!!
inline int audioEngine_updateNoteQueue( unsigned nFrames )
{
	static int nLastTick = -1;
	bool bSendPatternChange = false;

	unsigned int framepos;
	if (  m_audioEngineState == STATE_PLAYING ) {
		framepos = m_pAudioDriver->m_transport.m_nFrames;
	} else {
		// use this to support realtime events when not playing
		framepos = m_nRealtimeFrames;
	}

	int tickNumber_start = ( int )( framepos / m_pAudioDriver->m_transport.m_nTickSize );
	int tickNumber_end = ( int )( ( framepos + nFrames ) / m_pAudioDriver->m_transport.m_nTickSize );

	int tick = tickNumber_start;

	// get initial timestamp for first tick
	gettimeofday( &m_currentTickTime, NULL );

	while ( tick <= tickNumber_end ) {
		if ( tick == nLastTick ) {
			++tick;
			continue;
		} else {
			nLastTick = tick;
		}


		// midi events now get put into the m_songNoteQueue as well, based on their timestamp
		while ( m_midiNoteQueue.size() > 0 ) {
			Note *note = m_midiNoteQueue[0];

			if ( ( int )note->get_position() <= tick ) {
				// printf ("tick=%d  pos=%d\n", tick, note->getPosition());
				m_midiNoteQueue.pop_front();
				m_songNoteQueue.push_back( note );
			} else {
				break;
			}
		}

		if (  m_audioEngineState != STATE_PLAYING ) {
			// only keep going if we're playing
			continue;
		}


		// SONG MODE
		if ( m_pSong->get_mode() == Song::SONG_MODE ) {
			if ( m_pSong->get_pattern_group_vector()->size() == 0 ) {
				// there's no song!!
				_ERRORLOG( "no patterns in song." );
				m_pAudioDriver->stop();
				return -1;
			}

			m_nSongPos = findPatternInTick( tick, m_pSong->is_loop_enabled(), &m_nPatternStartTick );
			if ( m_nSongSizeInTicks != 0 ) {
				m_nPatternTickPosition = ( tick - m_nPatternStartTick ) % m_nSongSizeInTicks;
			} else {
				m_nPatternTickPosition = tick - m_nPatternStartTick;
			}

			if ( m_nPatternTickPosition == 0 ) {
				bSendPatternChange = true;
			}

			//PatternList *pPatternList = (*(m_pSong->getPatternGroupVector()))[m_nSongPos];
			if ( m_nSongPos == -1 ) {
				_INFOLOG( "song pos = -1" );
				if ( m_pSong->is_loop_enabled() == true ) {	// check if the song loop is enabled
					m_nSongPos = findPatternInTick( 0, true, &m_nPatternStartTick );
				} else {
					_INFOLOG( "End of Song" );
					return -1;
				}
			}
			PatternList *pPatternList = ( *( m_pSong->get_pattern_group_vector() ) )[m_nSongPos];
			// copio tutti i pattern
			m_pPlayingPatterns->clear();
			if ( pPatternList ) {
				for ( unsigned i = 0; i < pPatternList->get_size(); ++i ) {
					m_pPlayingPatterns->add( pPatternList->get( i ) );
				}
			}
		}

		// PATTERN MODE
		else if ( m_pSong->get_mode() == Song::PATTERN_MODE )	{
			//hydrogenInstance->warningLog( "pattern mode not implemented yet" );

			// per ora considero solo il primo pattern, se ce ne saranno piu' di uno
			// bisognera' prendere quello piu' piccolo

			int nPatternSize = MAX_NOTES;

			if ( m_pPlayingPatterns->get_size() != 0 ) {
				Pattern *pFirstPattern = m_pPlayingPatterns->get( 0 );
				nPatternSize = pFirstPattern->get_lenght();
			}

			if ( nPatternSize == 0 ) {
				_ERRORLOG( "nPatternSize == 0" );
			}

			if ( ( tick == m_nPatternStartTick + nPatternSize ) || ( m_nPatternStartTick == -1 ) ) {
				if ( m_pNextPatterns->get_size() > 0 ) {
					//hydrogenInstance->errorLog( "[audioEngine_updateNoteQueue] Aggiorno con nextpattern: " + to_string( (int)m_pNextPattern ) );
					/*					if ( m_bDeleteNextPattern ) {
											m_pPlayingPatterns->del( m_pNextPattern );
										}
										else {
											if ( m_bAppendNextPattern == false ) {
												m_pPlayingPatterns->clear();
											}
											m_pPlayingPatterns->add( m_pNextPattern );
										}*/
					_WARNINGLOG( "uh-oh, next patterns..." );
					Pattern * p;
					for ( uint i = 0; i < m_pNextPatterns->get_size(); i++ ) {
						p = m_pNextPatterns->get( i );
						_WARNINGLOG( "Got pattern #" + to_string( i+1 ) );
						// if the pattern isn't playing already, start it now.
						if ( ( m_pPlayingPatterns->del( p ) ) == NULL )
							m_pPlayingPatterns->add( p );
					}
					m_pNextPatterns->clear();
					bSendPatternChange = true;
				}
				m_nPatternStartTick = tick;
			}
			//m_nPatternTickPosition = tick % m_pCurrentPattern->getSize();
			m_nPatternTickPosition = tick % nPatternSize;
		}
		/*
					else {
						_ERRORLOG( "Pattern mode. m_pPlayingPatterns->getSize() = 0" );
						_ERRORLOG( "Panic! Stopping audio engine");
						// PANIC!
						m_pAudioDriver->stop();
					}
				}
		*/


		// metronome
		if (  ( m_nPatternStartTick == tick ) || ( ( tick - m_nPatternStartTick ) % 48 == 0 ) ) {
			float fPitch;
			float fVelocity;
			if ( m_nPatternTickPosition == 0 ) {
				fPitch = 3;
				fVelocity = 1.0;
				EventQueue::get_instance()->push_event( EVENT_METRONOME, 1 );
			} else {
				fPitch = 0;
				fVelocity = 0.8;
				EventQueue::get_instance()->push_event( EVENT_METRONOME, 0 );
			}
			if ( Preferences::getInstance()->m_bUseMetronome ) {
				m_pMetronomeInstrument->set_volume( Preferences::getInstance()->m_fMetronomeVolume );

				Note *pMetronomeNote = new Note( m_pMetronomeInstrument, tick, fVelocity, 0.5, 0.5, -1, fPitch );
				m_songNoteQueue.push_back( pMetronomeNote );
			}
		}



		// update the notes queue
		if ( m_pPlayingPatterns->get_size() != 0 ) {
			for ( unsigned nPat = 0; nPat < m_pPlayingPatterns->get_size(); ++nPat ) {
				Pattern *pPattern = m_pPlayingPatterns->get( nPat );
				assert( pPattern != NULL );

				std::multimap <int, Note*>::iterator pos;
				for ( pos = pPattern->note_map.lower_bound( m_nPatternTickPosition ); pos != pPattern->note_map.upper_bound( m_nPatternTickPosition ); ++pos ) {
					Note *pNote = pos->second;
					if ( pNote ) {
						unsigned nOffset = 0;

						// Swing
						float fSwingFactor = m_pSong->get_swing_factor();
						if ( ( ( m_nPatternTickPosition % 12 ) == 0 ) && ( ( m_nPatternTickPosition % 24 ) != 0 ) ) {	// da l'accento al tick 4, 12, 20, 36...
							nOffset += ( int )( ( 6.0 * m_pAudioDriver->m_transport.m_nTickSize ) * fSwingFactor );
						}

						// Humanize - Time parameter
						int nMaxTimeHumanize = 2000;
						if ( m_pSong->get_humanize_time_value() != 0 ) {
							nOffset += ( int )( getGaussian( 0.3 ) * m_pSong->get_humanize_time_value() * nMaxTimeHumanize );
						}
						//~

						Note *pCopiedNote = new Note( pNote );
						pCopiedNote->set_position( tick );

						pCopiedNote->m_nHumanizeDelay = nOffset;	// humanize time
						m_songNoteQueue.push_back( pCopiedNote );
						//pCopiedNote->dumpInfo();
					}
				}
			}
		}
		++tick;
	}

	// audioEngine_process must send the pattern change event after mutex unlock
	if ( bSendPatternChange ) {
		return 2;
	}
	return 0;
}



/// restituisce l'indice relativo al patternGroup in base al tick
inline int findPatternInTick( int nTick, bool bLoopMode, int *pPatternStartTick )
{
	assert( m_pSong );

	int nTotalTick = 0;
	m_nSongSizeInTicks = 0;

	std::vector<PatternList*> *pPatternColumns = m_pSong->get_pattern_group_vector();
	int nColumns = pPatternColumns->size();

	int nPatternSize;
	for ( int i = 0; i < nColumns; ++i ) {
		PatternList *pColumn = ( *pPatternColumns )[ i ];
		if ( pColumn->get_size() != 0 ) {
			// tengo in considerazione solo il primo pattern. I pattern nel gruppo devono avere la stessa lunghezza.
			nPatternSize = pColumn->get( 0 )->get_lenght();
		} else {
			nPatternSize = MAX_NOTES;
		}

		if ( ( nTick >= nTotalTick ) && ( nTick < nTotalTick + nPatternSize ) ) {
			( *pPatternStartTick ) = nTotalTick;
			return i;
		}
		nTotalTick += nPatternSize;
	}

	if ( bLoopMode ) {
		m_nSongSizeInTicks = nTotalTick;
		int nLoopTick = 0;
		if ( m_nSongSizeInTicks != 0 ) {
			nLoopTick = nTick % m_nSongSizeInTicks;
		}
		nTotalTick = 0;
		for ( int i = 0; i < nColumns; ++i ) {
			PatternList *pColumn = ( *pPatternColumns )[ i ];
			if ( pColumn->get_size() != 0 ) {
				// tengo in considerazione solo il primo pattern. I pattern nel gruppo devono avere la stessa lunghezza.
				nPatternSize = pColumn->get( 0 )->get_lenght();
			} else {
				nPatternSize = MAX_NOTES;
			}

			if ( ( nLoopTick >= nTotalTick ) && ( nLoopTick < nTotalTick + nPatternSize ) ) {
				( *pPatternStartTick ) = nTotalTick;
				return i;
			}
			nTotalTick += nPatternSize;
		}
	}

	char tmp[200];
	sprintf( tmp, "[findPatternInTick] tick = %d. No pattern found", nTick );
	_ERRORLOG( tmp );
	return -1;
}



void audioEngine_noteOn( Note *note )
{
	// check current state
	if ( ( m_audioEngineState != STATE_READY ) && ( m_audioEngineState != STATE_PLAYING ) ) {
		_ERRORLOG( "Error the audio engine is not in READY state" );
		delete note;
		return;
	}

	m_midiNoteQueue.push_back( note );
}



void audioEngine_noteOff( Note *note )
{
	if ( note == NULL )	{
		_ERRORLOG( "Error, note == NULL" );
	}

	AudioEngine::get_instance()->lock( "audioEngine_noteOff" );

	// check current state
	if ( ( m_audioEngineState != STATE_READY ) && ( m_audioEngineState != STATE_PLAYING ) ) {
		_ERRORLOG( "Error the audio engine is not in READY state" );
		delete note;
		AudioEngine::get_instance()->unlock();
		return;
	}

	/*
		for ( unsigned i = 0; i < m_playingNotesQueue.size(); ++i ) {	// delete old note
			Note *oldNote = m_playingNotesQueue[ i ];

			if ( oldNote->getInstrument() == note->getInstrument() ) {
				m_playingNotesQueue.erase( m_playingNotesQueue.begin() + i );
				delete oldNote;
				break;
			}
		}
	*/
	AudioEngine::get_instance()->get_sampler()->note_off( note );

	AudioEngine::get_instance()->unlock();

	delete note;
}



unsigned long audioEngine_getTickPosition()
{
	return m_nPatternTickPosition;
}


AudioOutput* createDriver( const std::string& sDriver )
{
	_INFOLOG( "Driver: \"" + sDriver + "\"" );
	Preferences *pPref = Preferences::getInstance();
	AudioOutput *pDriver = NULL;

	if ( sDriver == "Oss" ) {
		pDriver = new OssDriver( audioEngine_process );
		if ( pDriver->get_class_name() == "NullDriver" ) {
			delete pDriver;
			pDriver = NULL;
		}
	} else if ( sDriver == "Jack" ) {
		pDriver = new JackOutput( audioEngine_process );
		if ( pDriver->get_class_name() == "NullDriver" ) {
			delete pDriver;
			pDriver = NULL;
		} else {
#ifdef JACK_SUPPORT
			m_bUseDefaultOuts = pPref->m_bJackConnectDefaults;
			( ( JackOutput* ) pDriver )->setConnectDefaults( m_bUseDefaultOuts );
#endif
		}
	} else if ( sDriver == "Alsa" ) {
		pDriver = new AlsaAudioDriver( audioEngine_process );
		if ( pDriver->get_class_name() == "NullDriver" ) {
			delete pDriver;
			pDriver = NULL;
		}
	} else if ( sDriver == "PortAudio" ) {
		pDriver = new PortAudioDriver( audioEngine_process );
		if ( pDriver->get_class_name() == "NullDriver" ) {
			delete pDriver;
			pDriver = NULL;
		}
	}
//#ifdef Q_OS_MACX
	else if ( sDriver == "CoreAudio" ) {
		_INFOLOG( "Creating CoreAudioDriver" );
		pDriver = new CoreAudioDriver( audioEngine_process );
		if ( pDriver->get_class_name() == "NullDriver" ) {
			delete pDriver;
			pDriver = NULL;
		}
	}
//#endif
	else if ( sDriver == "Fake" ) {
		_WARNINGLOG( "*** Using FAKE audio driver ***" );
		pDriver = new FakeDriver( audioEngine_process );
	} else {
		_ERRORLOG( "Unknown driver " + sDriver );
		audioEngine_raiseError( Hydrogen::UNKNOWN_DRIVER );
	}

	if ( pDriver  ) {
		// initialize the audio driver
		int res = pDriver->init( pPref->m_nBufferSize );
		if ( res != 0 ) {
			_ERRORLOG( "Error starting audio driver [audioDriver::init()]" );
			delete pDriver;
			pDriver = NULL;
		}
	}

	return pDriver;
}


/// Start all audio drivers
void audioEngine_startAudioDrivers()
{
	Preferences *preferencesMng = Preferences::getInstance();

	AudioEngine::get_instance()->lock( "audioEngine_startAudioDrivers" );

	_INFOLOG( "[audioEngine_startAudioDrivers]" );

	// check current state
	if ( m_audioEngineState != STATE_INITIALIZED ) {
		_ERRORLOG( "Error the audio engine is not in INITIALIZED state. state=" + to_string( m_audioEngineState ) );
		AudioEngine::get_instance()->unlock();
		return;
	}
	if ( m_pAudioDriver ) {	// check if the audio m_pAudioDriver is still alive
		_ERRORLOG( "The audio driver is still alive" );
	}
	if ( m_pMidiDriver ) {	// check if midi driver is still alive
		_ERRORLOG( "The MIDI driver is still active" );
	}


	std::string sAudioDriver = preferencesMng->m_sAudioDriver;
//	sAudioDriver = "Auto";
	if ( sAudioDriver == "Auto" ) {
		if ( ( m_pAudioDriver = createDriver( "Jack" ) ) == NULL ) {
			if ( ( m_pAudioDriver = createDriver( "Alsa" ) ) == NULL ) {
				if ( ( m_pAudioDriver = createDriver( "CoreAudio" ) ) == NULL ) {
					if ( ( m_pAudioDriver = createDriver( "PortAudio" ) ) == NULL ) {
						if ( ( m_pAudioDriver = createDriver( "Oss" ) ) == NULL ) {
							audioEngine_raiseError( Hydrogen::ERROR_STARTING_DRIVER );
							_ERRORLOG( "Error starting audio driver" );
							_ERRORLOG( "Using the NULL output audio driver" );

							// use the NULL output driver
							m_pAudioDriver = new NullDriver( audioEngine_process );
							m_pAudioDriver->init( 0 );
						}
					}
				}
			}
		}
	} else {
		m_pAudioDriver = createDriver( sAudioDriver );
		if ( m_pAudioDriver == NULL ) {
			audioEngine_raiseError( Hydrogen::ERROR_STARTING_DRIVER );
			_ERRORLOG( "Error starting audio driver" );
			_ERRORLOG( "Using the NULL output audio driver" );

			// use the NULL output driver
			m_pAudioDriver = new NullDriver( audioEngine_process );
			m_pAudioDriver->init( 0 );
		}
	}

	if ( preferencesMng->m_sMidiDriver == "ALSA" ) {
#ifdef ALSA_SUPPORT
		// Create MIDI driver
		m_pMidiDriver = new AlsaMidiDriver();
		m_pMidiDriver->open();
		m_pMidiDriver->setActive( true );
#endif
	} else if ( preferencesMng->m_sMidiDriver == "PortMidi" ) {
#ifdef PORTMIDI_SUPPORT
		m_pMidiDriver = new PortMidiDriver();
		m_pMidiDriver->open();
		m_pMidiDriver->setActive( true );
#endif
	} else if ( preferencesMng->m_sMidiDriver == "CoreMidi" ) {
#ifdef COREMIDI_SUPPORT
		m_pMidiDriver = new CoreMidiDriver();
		m_pMidiDriver->open();
		m_pMidiDriver->setActive( true );
#endif
	}

	// change the current audio engine state
	if ( m_pSong == NULL ) {
		m_audioEngineState = STATE_PREPARED;
	} else {
		m_audioEngineState = STATE_READY;
	}


	if ( m_pSong ) {
		m_pAudioDriver->setBpm( m_pSong->__bpm );
	}

	// update the audiodriver reference in the sampler
	AudioEngine::get_instance()->get_sampler()->set_audio_output( m_pAudioDriver );

	if ( m_pAudioDriver ) {
		int res = m_pAudioDriver->connect();
		if ( res != 0 ) {
			audioEngine_raiseError( Hydrogen::ERROR_STARTING_DRIVER );
			_ERRORLOG( "Error starting audio driver [audioDriver::connect()]" );
			_ERRORLOG( "Using the NULL output audio driver" );

			delete m_pAudioDriver;
			m_pAudioDriver = new NullDriver( audioEngine_process );
			m_pAudioDriver->init( 0 );
			m_pAudioDriver->connect();
		}

		if ( ( m_pMainBuffer_L = m_pAudioDriver->getOut_L() ) == NULL ) {
			_ERRORLOG( "m_pMainBuffer_L == NULL" );
		}
		if ( ( m_pMainBuffer_R = m_pAudioDriver->getOut_R() ) == NULL ) {
			_ERRORLOG( "m_pMainBuffer_R == NULL" );
		}

#ifdef JACK_SUPPORT
		audioEngine_renameJackPorts();
#endif

		audioEngine_setupLadspaFX( m_pAudioDriver->getBufferSize() );
	}



	if ( m_audioEngineState == STATE_PREPARED ) {
		EventQueue::get_instance()->push_event( EVENT_STATE, STATE_PREPARED );
	} else if ( m_audioEngineState == STATE_READY ) {
		EventQueue::get_instance()->push_event( EVENT_STATE, STATE_READY );
	}

	AudioEngine::get_instance()->unlock();	// Unlocking earlier might execute the jack process() callback before we are fully initialized.
}



/// Stop all audio drivers
void audioEngine_stopAudioDrivers()
{
	_INFOLOG( "[audioEngine_stopAudioDrivers]" );

	// check current state
	if ( m_audioEngineState == STATE_PLAYING ) {
		audioEngine_stop();
	}

	if ( ( m_audioEngineState != STATE_PREPARED ) && ( m_audioEngineState != STATE_READY ) ) {
		_ERRORLOG( "Error: the audio engine is not in PREPARED or READY state. state=" + to_string( m_audioEngineState ) );
		return;
	}

	// delete MIDI driver
	if ( m_pMidiDriver ) {
		m_pMidiDriver->close();
		delete m_pMidiDriver;
		m_pMidiDriver = NULL;
	}

	AudioEngine::get_instance()->get_sampler()->set_audio_output( NULL );

	// delete audio driver
	if ( m_pAudioDriver ) {
		m_pAudioDriver->disconnect();
		delete m_pAudioDriver;
		m_pAudioDriver = NULL;
	}


	AudioEngine::get_instance()->lock( "audioEngine_stopAudioDrivers" );
	// change the current audio engine state
	m_audioEngineState = STATE_INITIALIZED;
	EventQueue::get_instance()->push_event( EVENT_STATE, STATE_INITIALIZED );
	AudioEngine::get_instance()->unlock();
}



/// Restart all audio and midi drivers
void audioEngine_restartAudioDrivers()
{
	audioEngine_stopAudioDrivers();
	audioEngine_startAudioDrivers();
}






//----------------------------------------------------------------------------
//
// Implementation of Hydrogen class
//
//----------------------------------------------------------------------------


Hydrogen* Hydrogen::instance = NULL;		/// static reference of Hydrogen class (Singleton)




Hydrogen::Hydrogen()
		: Object( "Hydrogen" )
{
	if ( instance ) {
		_ERRORLOG( "Hydrogen audio engine is already running" );
		throw H2Exception( "Hydrogen audio engine is already running" );
	}

	_INFOLOG( "[Hydrogen]" );

	hydrogenInstance = this;
	instance = this;
	audioEngine_init();
	audioEngine_startAudioDrivers();

}



Hydrogen::~Hydrogen()
{
	_INFOLOG( "[~Hydrogen]" );
	if ( m_audioEngineState == STATE_PLAYING ) {
		audioEngine_stop();
	}
	removeSong();
	audioEngine_stopAudioDrivers();
	audioEngine_destroy();
	instance = NULL;
}



/// Return the Hydrogen instance
Hydrogen* Hydrogen::get_instance()
{
	if ( instance == NULL ) {
		instance = new Hydrogen();
	}
	return instance;
}



/// Start the internal sequencer
void Hydrogen::sequencer_play()
{
	// play from start if pattern mode is enabled
	if ( m_pSong->get_mode() == Song::PATTERN_MODE ) {
		setPatternPos( 0 );
	}
	m_pAudioDriver->play();
}



/// Stop the internal sequencer
void Hydrogen::sequencer_stop()
{
	m_pAudioDriver->stop();
}



void Hydrogen::setSong( Song *pSong )
{
	audioEngine_setSong( pSong );
}



void Hydrogen::removeSong()
{
	audioEngine_removeSong();
}



Song* Hydrogen::getSong()
{
	return m_pSong;
}



void Hydrogen::midi_noteOn( Note *note )
{
	audioEngine_noteOn( note );
}



void Hydrogen::midi_noteOff( Note *note )
{
	audioEngine_noteOff( note );
}



void Hydrogen::addRealtimeNote( int instrument, float velocity, float pan_L, float pan_R, float pitch, bool forcePlay )
{
	UNUSED( pitch );

	Preferences *pref = Preferences::getInstance();
	unsigned int realcolumn = 0;
	unsigned res = pref->getPatternEditorGridResolution();
	int nBase = pref->isPatternEditorUsingTriplets() ? 3 : 4;
	int scalar = ( 4 * MAX_NOTES ) / ( res * nBase );
	bool hearnote = forcePlay;



	AudioEngine::get_instance()->lock( "Hydrogen::addRealtimeNote" );	// lock the audio engine

	Song *song = getSong();
	if ( instrument >= ( int )song->get_instrument_list()->get_size() ) {
		// unused instrument
		AudioEngine::get_instance()->unlock();
		return;
	}

	unsigned int column = getTickPosition();

	realcolumn = getRealtimeTickPosition();

	// quantize it to scale
	int qcolumn = ( int )::round( column / ( double )scalar ) * scalar;
	if ( qcolumn == MAX_NOTES ) qcolumn = 0;

	if ( pref->getQuantizeEvents() ) {
		column = qcolumn;
	}


	unsigned position = column;

	Pattern* currentPattern = NULL;
	PatternList *pPatternList = m_pSong->get_pattern_list();
	if ( ( m_nSelectedPatternNumber != -1 ) && ( m_nSelectedPatternNumber < ( int )pPatternList->get_size() ) ) {
		currentPattern = pPatternList->get( m_nSelectedPatternNumber );
	}

	Instrument *instrRef = 0;
	if ( song ) {
		instrRef = song->get_instrument_list()->get( instrument );
	}

	if ( currentPattern && ( getState() == STATE_PLAYING ) ) {
		bool bNoteAlreadyExist = false;
		for ( unsigned nNote = 0; nNote < currentPattern->get_lenght(); nNote++ ) {
			std::multimap <int, Note*>::iterator pos;
			for ( pos = currentPattern->note_map.lower_bound( nNote ); pos != currentPattern->note_map.upper_bound( nNote ); ++pos ) {
				Note *pNote = pos->second;
				if ( pNote!=NULL ) {
					if ( pNote->get_instrument() == instrRef && nNote==column ) {
						bNoteAlreadyExist = true;
						break;
					}
				}
			}
		}

		if ( bNoteAlreadyExist ) {
			// in this case, we'll leave the note alone
			// hear note only if not playing too
			if ( pref->getHearNewNotes() && getState() == STATE_READY ) {
				hearnote = true;
			}
		} else if ( !pref->getRecordEvents() ) {
			if ( pref->getHearNewNotes() && ( getState() == STATE_READY || getState() == STATE_PLAYING ) ) {
				hearnote = true;
			}
		} else {
			// create the new note
			Note *note = new Note( instrRef, position, velocity, pan_L, pan_R, -1, 0 );
			currentPattern->note_map.insert( std::make_pair( column, note ) );

			// hear note if its not in the future
			if ( pref->getHearNewNotes() && position <= getTickPosition() ) {
				hearnote = true;
			}

			song->__is_modified = true;

			EventQueue::get_instance()->push_event( EVENT_PATTERN_MODIFIED, -1 );
		}
	} else if ( pref->getHearNewNotes() ) {
		hearnote = true;
	}

	if ( hearnote && instrRef ) {
		Note *note2 = new Note( instrRef, realcolumn, velocity, pan_L, pan_R, -1, 0 );
		midi_noteOn( note2 );
	}

	AudioEngine::get_instance()->unlock(); // unlock the audio engine
}



float Hydrogen::getMasterPeak_L()
{
	return m_fMasterPeak_L;
}



float Hydrogen::getMasterPeak_R()
{
	return m_fMasterPeak_R;
}



unsigned long Hydrogen::getTickPosition()
{
	return audioEngine_getTickPosition();
}



unsigned long Hydrogen::getRealtimeTickPosition()
{
	//unsigned long initTick = audioEngine_getTickPosition();
	unsigned int initTick = ( unsigned int )( m_nRealtimeFrames / m_pAudioDriver->m_transport.m_nTickSize );
	unsigned long retTick;

	struct timeval currtime;
	struct timeval deltatime;

	double sampleRate = ( double ) m_pAudioDriver->getSampleRate();
	gettimeofday ( &currtime, NULL );

	timersub( &currtime, &m_currentTickTime, &deltatime );

	// add a buffers worth for jitter resistance
	double deltaSec = ( double ) deltatime.tv_sec + ( deltatime.tv_usec / 1000000.0 ) +  ( m_pAudioDriver->getBufferSize() / ( double )sampleRate );

	retTick = ( unsigned long ) ( ( sampleRate / ( double ) m_pAudioDriver->m_transport.m_nTickSize ) * deltaSec );

	retTick = initTick + retTick;

	return retTick;
}



PatternList* Hydrogen::getCurrentPatternList()
{
	return m_pPlayingPatterns;
}

PatternList * Hydrogen::getNextPatterns()
{
	return m_pNextPatterns;
}



/// Set the next pattern (Pattern mode only)
void Hydrogen::sequencer_setNextPattern( int pos, bool appendPattern, bool deletePattern )
{
	m_bAppendNextPattern = appendPattern;
	m_bDeleteNextPattern = deletePattern;

	AudioEngine::get_instance()->lock( "Hydrogen::sequencer_setNextPattern" );

	if ( m_pSong && m_pSong->get_mode() == Song::PATTERN_MODE ) {
		PatternList *patternList = m_pSong->get_pattern_list();
		Pattern * p = patternList->get( pos );
		if ( ( pos >= 0 ) && ( pos < ( int )patternList->get_size() ) ) {
			// if p is already on the next pattern list, delete it.
			if ( m_pNextPatterns->del( p ) == NULL ) {
// 				WARNINGLOG( "Adding to nextPatterns" );
				m_pNextPatterns->add( p );
			}/* else {
// 				WARNINGLOG( "Removing " + to_string(pos) );
			}*/
		} else {
			_ERRORLOG( "pos not in patternList range. pos=" + to_string( pos ) + " patternListSize=" + to_string( patternList->get_size() ) );
			m_pNextPatterns->clear();
		}
	} else {
		_ERRORLOG( "can't set next pattern in song mode" );
		m_pNextPatterns->clear();
	}

	AudioEngine::get_instance()->unlock();
}



int Hydrogen::getPatternPos()
{
	return m_nSongPos;
}



void Hydrogen::restartDrivers()
{
	audioEngine_restartAudioDrivers();
}



/// Export a song to a wav file, returns the elapsed time in mSec
void Hydrogen::startExportSong( const std::string& filename )
{
	if ( getState() == STATE_PLAYING ) {
		sequencer_stop();
	}
	Preferences *pPref = Preferences::getInstance();

	m_oldEngineMode = m_pSong->get_mode();
	m_bOldLoopEnabled = m_pSong->is_loop_enabled();

	m_pSong->set_mode( Song::SONG_MODE );
	m_pSong->set_loop_enabled( false );
	unsigned nSamplerate = m_pAudioDriver->getSampleRate();

	// stop all audio drivers
	audioEngine_stopAudioDrivers();

	/*
		FIXME: Questo codice fa davvero schifo....
	*/


	m_pAudioDriver = new DiskWriterDriver( audioEngine_process, nSamplerate, filename );

	AudioEngine::get_instance()->get_sampler()->stop_playing_notes();
	AudioEngine::get_instance()->get_sampler()->set_audio_output( m_pAudioDriver );

	// reset
	m_pAudioDriver->m_transport.m_nFrames = 0;	// reset total frames
	m_pAudioDriver->setBpm( m_pSong->__bpm );
	m_nSongPos = 0;
	m_nPatternTickPosition = 0;
	m_audioEngineState = STATE_PLAYING;
	m_nPatternStartTick = -1;

	int res = m_pAudioDriver->init( pPref->m_nBufferSize );
	if ( res != 0 ) {
		_ERRORLOG( "Error starting disk writer driver [DiskWriterDriver::init()]" );
	}

	m_pMainBuffer_L = m_pAudioDriver->getOut_L();
	m_pMainBuffer_R = m_pAudioDriver->getOut_R();

	audioEngine_setupLadspaFX( m_pAudioDriver->getBufferSize() );

	audioEngine_seek( 0, false );

	res = m_pAudioDriver->connect();
	if ( res != 0 ) {
		_ERRORLOG( "Error starting disk writer driver [DiskWriterDriver::connect()]" );
	}
}



void Hydrogen::stopExportSong()
{
	if ( m_pAudioDriver->get_class_name() != "DiskWriterDriver" ) {
		return;
	}

//	audioEngine_stopAudioDrivers();
	m_pAudioDriver->disconnect();

	m_audioEngineState = STATE_INITIALIZED;
	delete m_pAudioDriver;
	m_pAudioDriver = NULL;

	m_pMainBuffer_L = NULL;
	m_pMainBuffer_R = NULL;

	m_pSong->set_mode( m_oldEngineMode );
	m_pSong->set_loop_enabled( m_bOldLoopEnabled );

	m_nSongPos = -1;
	m_nPatternTickPosition = 0;
	audioEngine_startAudioDrivers();

	if ( m_pAudioDriver ) {
		m_pAudioDriver->setBpm( m_pSong->__bpm );
	} else {
		_ERRORLOG( "m_pAudioDriver = NULL" );
	}
}



/// Used to display audio driver info
AudioOutput* Hydrogen::getAudioOutput()
{
	return m_pAudioDriver;
}



/// Used to display midi driver info
MidiInput* Hydrogen::getMidiInput()
{
	return m_pMidiDriver;
}



void Hydrogen::setMasterPeak_L( float value )
{
	m_fMasterPeak_L = value;
}



void Hydrogen::setMasterPeak_R( float value )
{
	m_fMasterPeak_R = value;
}



int Hydrogen::getState()
{
	return m_audioEngineState;
}



void Hydrogen::setCurrentPatternList( PatternList *pPatternList )
{
	AudioEngine::get_instance()->lock( "Hydrogen::setCurrentPatternList" );
	m_pPlayingPatterns = pPatternList;
	EventQueue::get_instance()->push_event( EVENT_PATTERN_CHANGED, -1 );
	AudioEngine::get_instance()->unlock();
}



float Hydrogen::getProcessTime()
{
	return m_fProcessTime;
}



float Hydrogen::getMaxProcessTime()
{
	return m_fMaxProcessTime;
}



int Hydrogen::loadDrumkit( Drumkit *drumkitInfo )
{
	_INFOLOG( drumkitInfo->getName() );
	LocalFileMng fileMng;
	std::string sDrumkitPath = fileMng.getDrumkitDirectory( drumkitInfo->getName() );



	//current instrument list
	InstrumentList *songInstrList = m_pSong->get_instrument_list();
	
	//new instrument list
	InstrumentList *pDrumkitInstrList = drumkitInfo->getInstrumentList();

	/*
		If the old drumkit is bigger then the new drumkit,
		delete all instruments with a bigger pos then
		pDrumkitInstrList->get_size(). Otherwise the instruments
		from our old instrumentlist with
		pos > pDrumkitInstrList->get_size() stay in the 
		new instrumentlist
	*/

	while ( pDrumkitInstrList->get_size() < songInstrList->get_size() )
	{
		songInstrList->del(songInstrList->get_size() - 1);
	}

	for ( unsigned nInstr = 0; nInstr < pDrumkitInstrList->get_size(); ++nInstr ) {
		Instrument *pInstr = NULL;
		if ( nInstr < songInstrList->get_size() ) {
			//instrument exists already
			pInstr = songInstrList->get( nInstr );
			assert( pInstr );
		} else {
			pInstr = new Instrument( "", "", new ADSR() );
			AudioEngine::get_instance()->lock( "Hydrogen::loadDrumkit" );
			songInstrList->add( pInstr );
			AudioEngine::get_instance()->unlock();
		}

		//if ( pInstr->m_bIsLocked ) {	// is instrument locking still useful?
		//	continue;
		//}

		Instrument *pNewInstr = pDrumkitInstrList->get( nInstr );
		assert( pNewInstr );
		_INFOLOG( "Loading instrument (" + to_string( nInstr ) + " of " + to_string( pDrumkitInstrList->get_size() ) + ") [ " + pNewInstr->get_name() + " ]" );
		// creo i nuovi layer in base al nuovo strumento
		for ( unsigned nLayer = 0; nLayer < MAX_LAYERS; ++nLayer ) {
			InstrumentLayer *pNewLayer = pNewInstr->get_layer( nLayer );
			if ( pNewLayer != NULL ) {
				Sample *pNewSample = pNewLayer->get_sample();
				std::string sSampleFilename = sDrumkitPath + drumkitInfo->getName() + "/" + pNewSample->get_filename();
				_INFOLOG( "    |-> Loading layer [ " + sSampleFilename + " ]" );

				// carico il nuovo sample e creo il nuovo layer
				Sample *pSample = Sample::load( sSampleFilename );
//				pSample->setFilename( pNewSample->getFilename() );	// riuso il path del nuovo sample (perche' e' gia relativo al path del drumkit)
				InstrumentLayer *pOldLayer = pInstr->get_layer( nLayer );

				if ( pSample == NULL ) {
					//_ERRORLOG( "Error Loading drumkit: NULL sample, now using /emptySample.wav" );
					//pSample = Sample::load(string(DataPath::getDataPath() ).append( "/emptySample.wav" ));
					//pSample->m_sFilename = string(DataPath::getDataPath() ).append( "/emptySample.wav" );
					_ERRORLOG( "Error loading sample. Creating a new empty layer." );
					AudioEngine::get_instance()->lock( "Hydrogen::loadDrumkit" );
					pInstr->set_layer( NULL, nLayer );
					AudioEngine::get_instance()->unlock();
					delete pOldLayer;
					continue;
				}
				InstrumentLayer *pLayer = new InstrumentLayer( pSample );
				pLayer->set_start_velocity( pNewLayer->get_start_velocity() );
				pLayer->set_end_velocity( pNewLayer->get_end_velocity() );
				pLayer->set_gain( pNewLayer->get_gain() );

				AudioEngine::get_instance()->lock( "Hydrogen::loadDrumkit" );
				pInstr->set_layer( pLayer, nLayer );	// set the new layer
				AudioEngine::get_instance()->unlock();
				delete pOldLayer;		// delete the old layer

			} else {
				InstrumentLayer *pOldLayer = pInstr->get_layer( nLayer );
				AudioEngine::get_instance()->lock( "Hydrogen::loadDrumkit" );
				pInstr->set_layer( NULL, nLayer );
				AudioEngine::get_instance()->unlock();
				delete pOldLayer;		// delete the old layer
			}

		}
		AudioEngine::get_instance()->lock( "Hydrogen::loadDrumkit" );
		// update instrument properties
		pInstr->set_id( pNewInstr->get_id() );
		pInstr->set_name( pNewInstr->get_name() );
		pInstr->set_pan_l( pNewInstr->get_pan_l() );
		pInstr->set_pan_r( pNewInstr->get_pan_r() );
		pInstr->set_volume( pNewInstr->get_volume() );
		pInstr->set_drumkit_name( pNewInstr->get_drumkit_name() );
		pInstr->set_muted( pNewInstr->is_muted() );
		pInstr->set_random_pitch_factor( pNewInstr->get_random_pitch_factor() );
		pInstr->set_adsr( new ADSR( *( pNewInstr->get_adsr() ) ) );
		pInstr->set_filter_active( pNewInstr->is_filter_active() );
		pInstr->set_filter_cutoff( pNewInstr->get_filter_cutoff() );
		pInstr->set_filter_resonance( pNewInstr->get_filter_resonance() );
		pInstr->set_mute_group( pNewInstr->get_mute_group() );
		AudioEngine::get_instance()->unlock();
	}

	AudioEngine::get_instance()->lock( "Hydrogen::loadDrumkit" );
	audioEngine_renameJackPorts();
	AudioEngine::get_instance()->unlock();

	return 0;	//ok
}



void Hydrogen::raiseError( unsigned nErrorCode )
{
	audioEngine_raiseError( nErrorCode );
}


unsigned long Hydrogen::getTotalFrames()
{
	return m_pAudioDriver->m_transport.m_nFrames;
}

unsigned long Hydrogen::getRealtimeFrames()
{
	return m_nRealtimeFrames;
}

/**
 * Get the ticks for pattern at pattern pos
 * @a int pos -- position in song
 * @return -1 if pos > number of patterns in the song, tick no. > 0 otherwise
 * The driver should be LOCKED when calling this!!
 */
long Hydrogen::getTickForPosition( int pos )
{
	int nPatternGroups = m_pSong->get_pattern_group_vector()->size();
	if ( pos >= nPatternGroups ) {
		if ( m_pSong->is_loop_enabled() ) {
			pos = pos % nPatternGroups;
		} else {
			_WARNINGLOG( "patternPos > nPatternGroups. pos: " + to_string( pos ) + ", nPatternGroups: " + to_string( nPatternGroups ) );
			return -1;
		}
	}

	std::vector<PatternList*> *pColumns = m_pSong->get_pattern_group_vector();
	long totalTick = 0;
	int nPatternSize;
	Pattern *pPattern = NULL;
	for ( int i = 0; i < pos; ++i ) {
		PatternList *pColumn = ( *pColumns )[ i ];
		pPattern = pColumn->get( 0 );	// prendo solo il primo. I pattern nel gruppo devono avere la stessa lunghezza
		if ( pPattern ) {
			nPatternSize = pPattern->get_lenght();
		} else {
			nPatternSize = MAX_NOTES;
		}

		totalTick += nPatternSize;
	}
	return totalTick;
}

/// Set the position in the song
void Hydrogen::setPatternPos( int pos )
{
	AudioEngine::get_instance()->lock( "Hydrogen::setPatternPos" );

	long totalTick = getTickForPosition( pos );
	if ( totalTick < 0 ) {
		AudioEngine::get_instance()->unlock();
		return;
	}

	if ( getState() != STATE_PLAYING ) {
		// find pattern immediately when not playing
		int dummy;
		m_nSongPos = findPatternInTick( totalTick, m_pSong->is_loop_enabled(), &dummy );
	}

	m_pAudioDriver->locate( ( int ) ( totalTick * m_pAudioDriver->m_transport.m_nTickSize ) );


	AudioEngine::get_instance()->unlock();
}




void Hydrogen::getLadspaFXPeak( int nFX, float *fL, float *fR )
{
#ifdef LADSPA_SUPPORT
	( *fL ) = m_fFXPeak_L[nFX];
	( *fR ) = m_fFXPeak_R[nFX];
#else
	( *fL ) = 0;
	( *fR ) = 0;
#endif
}



void Hydrogen::setLadspaFXPeak( int nFX, float fL, float fR )
{
#ifdef LADSPA_SUPPORT
	m_fFXPeak_L[nFX] = fL;
	m_fFXPeak_R[nFX] = fR;
#endif
}



void Hydrogen::setTapTempo( float fInterval )
{

//	infoLog( "set tap tempo" );
	static float fOldBpm1 = -1;
	static float fOldBpm2 = -1;
	static float fOldBpm3 = -1;
	static float fOldBpm4 = -1;
	static float fOldBpm5 = -1;
	static float fOldBpm6 = -1;
	static float fOldBpm7 = -1;
	static float fOldBpm8 = -1;

	float fBPM = 60000.0 / fInterval;

	if ( fabs( fOldBpm1 - fBPM ) > 20 ) {	// troppa differenza, niente media
		fOldBpm1 = fBPM;
		fOldBpm2 = fBPM;
		fOldBpm3 = fBPM;
		fOldBpm4 = fBPM;
		fOldBpm5 = fBPM;
		fOldBpm6 = fBPM;
		fOldBpm7 = fBPM;
		fOldBpm8 = fBPM;
	}

	if ( fOldBpm1 == -1 ) {
		fOldBpm1 = fBPM;
		fOldBpm2 = fBPM;
		fOldBpm3 = fBPM;
		fOldBpm4 = fBPM;
		fOldBpm5 = fBPM;
		fOldBpm6 = fBPM;
		fOldBpm7 = fBPM;
		fOldBpm8 = fBPM;
	}

	fBPM = ( fBPM + fOldBpm1 + fOldBpm2 + fOldBpm3 + fOldBpm4 + fOldBpm5 + fOldBpm6 + fOldBpm7 + fOldBpm8 ) / 9.0;


	_INFOLOG( "avg BPM = " + to_string( fBPM ) );
	fOldBpm8 = fOldBpm7;
	fOldBpm7 = fOldBpm6;
	fOldBpm6 = fOldBpm5;
	fOldBpm5 = fOldBpm4;
	fOldBpm4 = fOldBpm3;
	fOldBpm3 = fOldBpm2;
	fOldBpm2 = fOldBpm1;
	fOldBpm1 = fBPM;

	AudioEngine::get_instance()->lock( "Hydrogen::setTapTempo" );

// 	m_pAudioDriver->setBpm( fBPM );
// 	m_pSong->setBpm( fBPM );

	setBPM( fBPM );

	AudioEngine::get_instance()->unlock();
}


// Called with audioEngine in LOCKED state.
void Hydrogen::setBPM( float fBPM )
{
	if ( m_pAudioDriver && m_pSong ) {
		m_pAudioDriver->setBpm( fBPM );
		m_pSong->__bpm = fBPM;
//		audioEngine_process_checkBPMChanged();
	}
}



void Hydrogen::restartLadspaFX()
{
	if ( m_pAudioDriver ) {
		AudioEngine::get_instance()->lock( "Hydrogen::restartLadspaFX" );
		audioEngine_setupLadspaFX( m_pAudioDriver->getBufferSize() );
		AudioEngine::get_instance()->unlock();
	} else {
		_ERRORLOG( "m_pAudioDriver = NULL" );
	}
}



int Hydrogen::getSelectedPatternNumber()
{
	return m_nSelectedPatternNumber;
}



void Hydrogen::setSelectedPatternNumber( int nPat )
{
	// FIXME: controllare se e' valido..
	if ( nPat == m_nSelectedPatternNumber )	return;

	m_nSelectedPatternNumber = nPat;

	EventQueue::get_instance()->push_event( EVENT_SELECTED_PATTERN_CHANGED, -1 );
}



int Hydrogen::getSelectedInstrumentNumber()
{
	return m_nSelectedInstrumentNumber;
}



void Hydrogen::setSelectedInstrumentNumber( int nInstrument )
{
	if ( m_nSelectedInstrumentNumber == nInstrument )	return;

	m_nSelectedInstrumentNumber = nInstrument;
	EventQueue::get_instance()->push_event( EVENT_SELECTED_INSTRUMENT_CHANGED, -1 );
}

#ifdef JACK_SUPPORT
void Hydrogen::renameJackPorts()
{
	audioEngine_renameJackPorts();
}
#endif

bool Hydrogen::handleAction( action *pAction )
{
	Hydrogen *pEngine = Hydrogen::get_instance();

	switch( pAction->getType())
	{
		case PLAY:
		{
			int nState = pEngine->getState();
			switch (nState) 
			{
				case STATE_READY:
					pEngine->sequencer_play();
					break;

				case STATE_PLAYING:
					pEngine->sequencer_stop();
					break;

				default:
					ERRORLOG( "[Hydrogen::actionHandler(PLAY): Unhandled case" );
			}
			break;
		}

		case STOP:
			pEngine->sequencer_stop();
			pEngine->setPatternPos( 0 );
			break;
			
	}
	return true;
}

};


