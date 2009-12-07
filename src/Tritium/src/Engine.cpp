/*
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 * Copyright(c) 2009 by Gabriel M. Beddingfield <gabriel@teuton.org>
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

/*
 In redesigning the Sampler, the following responsibilities were
 shifted over to the Sequencer (Tritium::Engine):

   o Must explicitly schedule Note On/Off events.  If Off event
     omitted, the note will stop when the sample ends.

   o Must supply a valid TransportPosition.

   o SeqEvent::frame is always relative to the current process()
     cycle.

   o in Sampler::process(beg, end, pos, nFrames), beg and end
     must be for this process() cycle only.  It will not be
     checked.

   o Sequencer is responsible for all scheduling the effects of all
     humanize, lead/lag, et al features.

   o It is undefined yet what to do for sample preview.  People need
     some level of access to Sampler in an "anytime, anywhere"
     fashion.  However, ATM it is not thread safe.  Currently, Sampler
     has no mutexes or any other kind of lock.  I'd like to keep it
     this way.  But this may mean that all "preview" features need to
     be handled by the Sequencer somehow.

 */

#include "config.h"

#ifdef WIN32
#    include <Tritium/timehelper.hpp>
#else
#    include <unistd.h>
#    include <sys/time.h>
#endif

#include <cassert>
#include <cstdio>
#include <deque>
#include <queue>
#include <list>
#include <iostream>
#include <ctime>
#include <cmath>

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include <Tritium/Logger.hpp>
#include <Tritium/LocalFileMng.hpp>
#include <Tritium/EventQueue.hpp>
#include <Tritium/ADSR.hpp>
#include <Tritium/SoundLibrary.hpp>
#include <Tritium/H2Exception.hpp>
#include <Tritium/AudioEngine.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/InstrumentLayer.hpp>
#include <Tritium/InstrumentList.hpp>
#include <Tritium/Sample.hpp>
#include <Tritium/Engine.hpp>
#include <Tritium/Pattern.hpp>
#include <Tritium/Note.hpp>
#include <Tritium/Action.hpp>
#include <Tritium/fx/LadspaFX.hpp>
#include <Tritium/fx/Effects.hpp>
#include <Tritium/IO/AudioOutput.hpp>
#include <Tritium/IO/JackOutput.hpp>
#include <Tritium/IO/NullDriver.hpp>
#include <Tritium/IO/MidiInput.hpp>
#include <Tritium/Preferences.hpp>
#include <Tritium/DataPath.hpp>
#include <Tritium/Sampler.hpp>
#include <Tritium/MidiMap.hpp>
#include <Tritium/Playlist.hpp>

#include <Tritium/Transport.hpp>
#include "transport/H2Transport.hpp"
#include <Tritium/SeqEvent.hpp>
#include <Tritium/SeqScript.hpp>
#include <Tritium/SeqScriptIterator.hpp>
#include "BeatCounter.hpp"
#include "SongSequencer.hpp"

#include "IO/FakeDriver.hpp"
#include "IO/DiskWriterDriver.hpp"
#include "IO/JackMidiDriver.hpp"
#include "IO/JackClient.hpp"

namespace Tritium
{

/**
 * This class provides a thread-safe queue that can be written from
 * anywhere to allow note on/off events.  It's primarily intended for
 * GUI input, and not something like a MIDI input.  (However, MIDI
 * input will probably use it temporarily.
 *
 * It provides a process() method that allows the events to be given
 * to the master sequencer queue.
 */
class GuiInputQueue
{
private:
	typedef std::list<SeqEvent> EvList;
	EvList __events;
	QMutex __mutex;

public:
	int process( SeqScript& seq, const TransportPosition& pos, uint32_t nframes ) {
		// Set up quantization.
		uint32_t quant_frame;

		{
			// TODO:  This seems too complicated for what we're doing...
			Preferences *pref = Engine::get_instance()->get_preferences();
			TransportPosition quant(pos);
			quant.ceil(TransportPosition::TICK);

			double res = (double)pref->getPatternEditorGridResolution();
			double trip_f = pref->isPatternEditorUsingTriplets() ? (2.0/3.0) : 1.0; // Triplet factor
			double fquant_ticks = quant.ticks_per_beat * (4.0 / res) * trip_f;  // Round to scalar * beat resolution
			int quant_ticks = round(fquant_ticks) - quant.tick;
			if( quant_ticks > 0 ) {
				quant += quant_ticks;
			}

			quant_frame = quant.frame - pos.frame;
		}

		// Add events to 'seq'
		QMutexLocker mx(&__mutex);
		EvList::iterator k;
		for( k=__events.begin() ; k!=__events.end() ; ++k ) {
			if( k->quantize ) {
				k->frame = quant_frame;
			}
			seq.insert(*k);
		}
		__events.clear();
		return 0;
	}

	void note_on( const Note* pNote, bool quantize = false ) {
		SeqEvent ev;
		QMutexLocker mx(&__mutex);
		ev.frame = 0;
		ev.type = SeqEvent::NOTE_ON;
		ev.note = *pNote;
		ev.quantize = quantize;
		ev.instrument_index =
			Engine::get_instance()->getSong()
			->get_instrument_list()->get_pos( pNote->get_instrument() );
		__events.push_back(ev);
	}

	void note_off( const Note* pNote, bool quantize = false ) {
		SeqEvent ev;
		QMutexLocker mx(&__mutex);
		ev.frame = 0;
		ev.type = SeqEvent::NOTE_OFF;
		ev.note = *pNote;
		ev.quantize = quantize;
		ev.instrument_index =
			Engine::get_instance()->getSong()
			->get_instrument_list()->get_pos( pNote->get_instrument() );
		__events.push_back(ev);
	}

	void panic() {
		SeqEvent ev;
		QMutexLocker mx(&__mutex);
		__events.clear();
		ev.frame = 0;
		ev.type = SeqEvent::ALL_OFF;
		ev.instrument_index = 0;
		__events.push_front(ev);
	}

	void clear() {
		QMutexLocker mx(&__mutex);
		__events.clear();
	}

};

// GLOBALS

/************************
 * DEAD VARIABLES 
 ************************
 * These are variables that are in the process of
 * being removed in the transport redesign.
 */
#if 0
//jack time master
float m_nNewBpmJTM = 120;
unsigned long m_nHumantimeFrames = 0;
//~ jack time master
int m_nPatternStartTick = -1;
int m_nPatternTickPosition = 0;
int m_nLookaheadFrames = 0;

// used in findPatternInTick
int m_nSongSizeInTicks = 0;

struct timeval m_currentTickTime;

unsigned long m_nRealtimeFrames = 0;
unsigned m_nBufferSize = 0;

                                                               /// Song Note FIFO
std::priority_queue<Note*, std::deque<Note*>, compare_pNotes > m_songNoteQueue;
std::deque<Note*> m_midiNoteQueue;	///< Midi Note FIFO

PatternList* m_pNextPatterns;		///< Next pattern (used only in Pattern mode)
bool m_bAppendNextPattern;		///< Add the next pattern to the list instead
					/// of replace.
bool m_bDeleteNextPattern;		///< Delete the next pattern from the list.


PatternList* m_pPlayingPatterns;
int m_nSongPos;				///< Is the position inside the song

#endif // 0
/****** END OF DEAD VARIABLES ********/

// info
float m_fMasterPeak_L = 0.0f;		///< Master peak (left channel)
float m_fMasterPeak_R = 0.0f;		///< Master peak (right channel)
float m_fProcessTime = 0.0f;		///< time used in process function
float m_fMaxProcessTime = 0.0f;		///< max ms usable in process with no xrun
//~ info

Preferences* m_preferences = 0;
ActionManager* m_action_manager = 0;
AudioEngine* m_audio_engine = 0;
EventQueue* m_event_queue = 0;
H2Transport* m_pTransport = 0;
Playlist* m_playlist = 0;
#ifdef JACK_SUPPORT
JackClient* m_jack_client = 0;
#endif
#ifdef LADSPA_SUPPORT
Effects* m_effects = 0;
#endif

// This is *the* priority queue for scheduling notes/events to be
// sent to the Sampler.
SeqScript m_queue;
GuiInputQueue m_GuiInput;
SongSequencer m_SongSequencer;

BeatCounter m_BeatCounter;

AudioOutput *m_pAudioDriver = NULL;	///< Audio output
MidiInput *m_pMidiDriver = NULL;	///< MIDI input
QMutex mutex_OutputPointer;		///< Mutex for audio output pointer, allows multiple readers
                                        ///< When locking this AND AudioEngine, always lock AudioEngine first.


Song *m_pSong;				///< Current song
Instrument *m_pMetronomeInstrument = NULL;	///< Metronome instrument
unsigned long m_nFreeRollingFrameCounter;

// Buffers used in the process function
float *m_pMainBuffer_L = NULL;
float *m_pMainBuffer_R = NULL;

Engine* engineInstance = NULL;   ///< Engine class instance (used for log)

int  m_audioEngineState = STATE_UNINITIALIZED;	///< Audio engine state

int m_nSelectedPatternNumber;
int m_nSelectedInstrumentNumber;
bool m_sendPatternChange;

#ifdef LADSPA_SUPPORT
float m_fFXPeak_L[MAX_FX];
float m_fFXPeak_R[MAX_FX];
#endif

// PROTOTYPES
void	audioEngine_init();
void	audioEngine_destroy();
int	audioEngine_start( bool bLockEngine = false, unsigned nTotalFrames = 0 );
void	audioEngine_stop( bool bLockEngine = false );
void	audioEngine_setSong( Song *newSong );
void	audioEngine_removeSong();
static void	audioEngine_noteOn( Note *note );
static void	audioEngine_noteOff( Note *note );
int	audioEngine_process( uint32_t nframes, void *arg );
inline void audioEngine_clearNoteQueue();
inline void audioEngine_process_playNotes( unsigned long nframes );

inline unsigned audioEngine_renderNote( Note* pNote, const unsigned& nBufferSize );
    inline void audioEngine_updateNoteQueue( unsigned nFrames, const TransportPosition& pos );
inline void audioEngine_prepNoteQueue();

inline int findPatternInTick( int tick, bool loopMode, int *patternStartTick );

void audioEngine_restartAudioDrivers();
void audioEngine_startAudioDrivers();
void audioEngine_stopAudioDrivers();

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
	return x1 * w * z + 0.0; // tunable
}



void audioEngine_raiseError( unsigned nErrorCode )
{
	Engine::get_instance()->get_event_queue()->push_event( EVENT_ERROR, nErrorCode );
}


/*
void updateTickSize()
{
	float sampleRate = ( float )m_pAudioDriver->getSampleRate();
	m_pAudioDriver->m_transport.m_nTickSize =
		( sampleRate * 60.0 /  m_pSong->__bpm / m_pSong->__resolution );
}
*/

void audioEngine_init()
{
	INFOLOG( "*** Engine audio engine init ***" );

	// check current state
	if ( m_audioEngineState != STATE_UNINITIALIZED ) {
		ERRORLOG( "Error the audio engine is not in UNINITIALIZED state" );
		Engine::get_instance()->get_audio_engine()->unlock();
		return;
	}

	m_nFreeRollingFrameCounter = 0;
	m_pSong = NULL;
	m_nSelectedPatternNumber = 0;
	m_nSelectedInstrumentNumber = 0;
	m_pMetronomeInstrument = NULL;
	m_pAudioDriver = NULL;

	m_pMainBuffer_L = NULL;
	m_pMainBuffer_R = NULL;

	srand( time( NULL ) );

	// Create metronome instrument
	QString sMetronomeFilename = QString( "%1/click.wav" )
					.arg( DataPath::get_data_path() );
	m_pMetronomeInstrument =
		new Instrument( sMetronomeFilename, "metronome", new ADSR() );
	m_pMetronomeInstrument->set_layer(
		new InstrumentLayer( Sample::load( sMetronomeFilename ) ),
		0
		);

	// Change the current audio engine state
	m_audioEngineState = STATE_INITIALIZED;

#ifdef JACK_SUPPORT
	m_jack_client = new JackClient(false);
#endif
	m_audio_engine = new AudioEngine();
#ifdef LADSPA_SUPPORT
	m_effects = new Effects();
#endif
	m_playlist = new Playlist();

	Engine::get_instance()->get_event_queue()->push_event( EVENT_STATE, STATE_INITIALIZED );

}



void audioEngine_destroy()
{
	// check current state
	if ( m_audioEngineState != STATE_INITIALIZED ) {
		ERRORLOG( "Error the audio engine is not in INITIALIZED state" );
		return;
	}
	Engine::get_instance()->get_audio_engine()->get_sampler()->panic();

	Engine::get_instance()->get_audio_engine()->lock( RIGHT_HERE );
	INFOLOG( "*** Engine audio engine shutdown ***" );

	audioEngine_clearNoteQueue();

	// change the current audio engine state
	m_audioEngineState = STATE_UNINITIALIZED;
	Engine::get_instance()->get_event_queue()->push_event( EVENT_STATE, STATE_UNINITIALIZED );

	delete m_pMetronomeInstrument;
	m_pMetronomeInstrument = NULL;

	Engine::get_instance()->get_audio_engine()->unlock();
#ifdef LADSPA_SUPPORT
	delete m_effects;
#endif
	delete m_audio_engine;
#ifdef JACK_SUPPORT
	delete m_jack_client;
#endif
}





/// Start playing
/// return 0 = OK
/// return -1 = NULL Audio Driver
/// return -2 = Driver connect() error
int audioEngine_start( bool bLockEngine, unsigned nTotalFrames )
{
	if ( bLockEngine ) {
		Engine::get_instance()->get_audio_engine()->lock( RIGHT_HERE );
	}

	INFOLOG( "[audioEngine_start]" );

	// check current state
	if ( m_audioEngineState != STATE_READY ) {
		ERRORLOG( "Error the audio engine is not in READY state" );
		if ( bLockEngine ) {
			Engine::get_instance()->get_audio_engine()->unlock();
		}
		return 0;	// FIXME!!
	}

	m_fMasterPeak_L = 0.0f;
	m_fMasterPeak_R = 0.0f;
	/*
	m_pAudioDriver->m_transport.m_nFrames = nTotalFrames;	// reset total frames
	m_nSongPos = -1;
	m_nPatternStartTick = -1;
	m_nPatternTickPosition = 0;

	// prepare the tickSize for this song
	updateTickSize();

	*/
	m_pTransport->start();

	if ( bLockEngine ) {
		Engine::get_instance()->get_audio_engine()->unlock();
	}
	return 0; // per ora restituisco sempre OK
}



/// Stop the audio engine
void audioEngine_stop( bool bLockEngine )
{
	if ( bLockEngine ) {
		Engine::get_instance()->get_audio_engine()->lock( RIGHT_HERE );
	}
	INFOLOG( "[audioEngine_stop]" );

	// check current state
	if ( m_audioEngineState != STATE_READY ) {
		ERRORLOG( "Error the audio engine is not in READY state, can't stop." );
		if ( bLockEngine ) {
			Engine::get_instance()->get_audio_engine()->unlock();
		}
		return;
	}

	// change the current audio engine state
	/*
	m_audioEngineState = STATE_READY;
	*/
	m_pTransport->stop();
	Engine::get_instance()->get_event_queue()->push_event( EVENT_STATE, STATE_READY );

	m_fMasterPeak_L = 0.0f;
	m_fMasterPeak_R = 0.0f;

	audioEngine_clearNoteQueue();

	if ( bLockEngine ) {
		Engine::get_instance()->get_audio_engine()->unlock();
	}
}

#if 0
//// THIS IS DEAD CODE.  BUT, I NEED TO REFER TO IT
//// WHEN RE-IMPLEMENTING LOOKAHEAD AND RANDOM PITCH.
inline void audioEngine_process_playNotes( unsigned long nframes )
{
	unsigned int framepos;

	if (  m_audioEngineState == STATE_PLAYING ) {
		framepos = m_pAudioDriver->m_transport.m_nFrames;
	} else {
		// use this to support realtime events when not playing
		framepos = m_nRealtimeFrames;
	}

	// reading from m_songNoteQueue
	while ( !m_songNoteQueue.empty() ) {
		Note *pNote = m_songNoteQueue.top();

		// verifico se la nota rientra in questo ciclo
		unsigned int noteStartInFrames =
			(int)( pNote->get_position() * m_pAudioDriver->m_transport.m_nTickSize );

		// if there is a negative Humanize delay, take into account so
		// we don't miss the time slice.  ignore positive delay, or we
		// might end the queue processing prematurely based on NoteQueue
		// placement.  the sampler handles positive delay.
		if (pNote->m_nHumanizeDelay < 0) {
			noteStartInFrames += pNote->m_nHumanizeDelay;
		}

		// m_nTotalFrames <= NotePos < m_nTotalFrames + bufferSize
		bool isNoteStart = ( ( noteStartInFrames >= framepos )
				     && ( noteStartInFrames < ( framepos + nframes ) ) );
		bool isOldNote = noteStartInFrames < framepos;
		if ( isNoteStart || isOldNote ) {

			// Humanize - Velocity parameter
			if ( m_pSong->get_humanize_velocity_value() != 0 ) {
				float random = m_pSong->get_humanize_velocity_value()
					       * getGaussian( 0.2 );
				pNote->set_velocity(
					pNote->get_velocity()
					+ ( random
					    - ( m_pSong->get_humanize_velocity_value() / 2.0 ) ) 
					);
				if ( pNote->get_velocity() > 1.0 ) {
					pNote->set_velocity( 1.0 );
				} else if ( pNote->get_velocity() < 0.0 ) {
					pNote->set_velocity( 0.0 );
				}
			}

			// Random Pitch ;)
			const float fMaxPitchDeviation = 2.0;
			pNote->set_pitch( pNote->get_pitch()
					  + ( fMaxPitchDeviation * getGaussian( 0.2 )
					      - fMaxPitchDeviation / 2.0 )
					  * pNote->get_instrument()->get_random_pitch_factor() );

///new note off stuff
//not in use for the moment, but it works! i am planing a bit more than only delete the note. better way is that
//users can edit the sustain-curve to fade out the sample.
//more details see sampler.cpp: Sampler::note_off( Note* note )

			//stop note bevore playing new note, only if set into the planned instrumenteditor checkbox `always stop note`
			//Instrument * noteInstrument = pNote->get_instrument();
			//if ( noteInstrument->is_stop_notes() ){ 
			//	Engine::get_instance()->get_audio_engine()->get_sampler()->note_off( pNote );
			//}
///~new note off stuff

			// aggiungo la nota alla lista di note da eseguire
			Engine::get_instance()->get_audio_engine()->get_sampler()->note_on( pNote );
			
			m_songNoteQueue.pop(); // rimuovo la nota dalla lista di note
			pNote->get_instrument()->dequeue();
			// raise noteOn event
			int nInstrument = m_pSong->get_instrument_list()
					         ->get_pos( pNote->get_instrument() );
			Engine::get_instance()->get_event_queue()->push_event( EVENT_NOTEON, nInstrument );
			continue;
		} else {
			// this note will not be played
			break;
		}
	}
}
#endif // 0

void audioEngine_clearNoteQueue()
{
	m_queue.clear();
	m_GuiInput.clear();
	Engine::get_instance()->get_audio_engine()->get_sampler()->panic();
}

/// Clear all audio buffers
inline void audioEngine_process_clearAudioBuffers( uint32_t nFrames )
{
	QMutexLocker mx( &mutex_OutputPointer );

	// clear main out Left and Right
	if ( m_pAudioDriver ) {
		m_pMainBuffer_L = m_pAudioDriver->getOut_L();
		m_pMainBuffer_R = m_pAudioDriver->getOut_R();
	} else {
		m_pMainBuffer_L = m_pMainBuffer_R = 0;
	}
	if ( m_pMainBuffer_L ) {
		memset( m_pMainBuffer_L, 0, nFrames * sizeof( float ) );
	}
	if ( m_pMainBuffer_R ) {
		memset( m_pMainBuffer_R, 0, nFrames * sizeof( float ) );
	}

#ifdef JACK_SUPPORT
	JackOutput* jo = dynamic_cast<JackOutput*>(m_pAudioDriver);
	if( jo && jo->has_track_outs() ) {
		float* buf;
		int k;
		for( k=0 ; k<jo->getNumTracks() ; ++k ) {
			buf = jo->getTrackOut_L(k);
			if( buf ) {
				memset( buf, 0, nFrames * sizeof( float ) );
			}
			buf = jo->getTrackOut_R(k);
			if( buf ) {
				memset( buf, 0, nFrames * sizeof( float ) );
			}
		}
	}
#endif

	mx.unlock();

#ifdef LADSPA_SUPPORT
	if ( m_audioEngineState >= STATE_READY ) {
		Effects* pEffects = Engine::get_instance()->get_effects();
		for ( unsigned i = 0; i < MAX_FX; ++i ) {	// clear FX buffers
			LadspaFX* pFX = pEffects->getLadspaFX( i );
			if ( pFX ) {
				assert( pFX->m_pBuffer_L );
				assert( pFX->m_pBuffer_R );
				memset( pFX->m_pBuffer_L, 0, nFrames * sizeof( float ) );
				memset( pFX->m_pBuffer_R, 0, nFrames * sizeof( float ) );
			}
		}
	}
#endif
}

/// Main audio processing function. Called by audio drivers.
int audioEngine_process( uint32_t nframes, void* /*arg*/ )
{
	timeval startTimeval = currentTime2();
	m_nFreeRollingFrameCounter += nframes;

	audioEngine_process_clearAudioBuffers( nframes );

	if( m_audioEngineState < STATE_READY) {
		return 0;
	}

	// Hook for MIDI in-process callbacks.  It calls its own locks
	// on the audioengine
	if (m_pMidiDriver) m_pMidiDriver->processAudio(nframes);

	Engine::get_instance()->get_audio_engine()->lock( RIGHT_HERE );

	if( m_audioEngineState < STATE_READY) {
		Engine::get_instance()->get_audio_engine()->unlock();
		return 0;
	}

        Transport* xport = Engine::get_instance()->get_transport();
        TransportPosition pos;
        xport->get_position(&pos);

	// PROCESS ALL INPUT SOURCES
	m_GuiInput.process(m_queue, pos, nframes);
	#warning "TODO: get MidiDriver::process() in the mix."
	// TODO: m_pMidiDriver->process(m_queue, pos, nframes);
	m_SongSequencer.process(m_queue, pos, nframes, m_sendPatternChange);

	// PROCESS ALL OUTPUTS


	/*
	// always update note queue.. could come from pattern or realtime input
	// (midi, keyboard)
	audioEngine_updateNoteQueue( nframes, pos );
	audioEngine_process_playNotes( nframes );
	*/

	// SAMPLER
	Sampler* pSampler = Engine::get_instance()->get_audio_engine()->get_sampler();
	pSampler->process( m_queue.begin_const(),
			   m_queue.end_const(nframes),
			   pos,
			   nframes
	    );
	float* out_L = Engine::get_instance()->get_audio_engine()->get_sampler()->__main_out_L;
	float* out_R = Engine::get_instance()->get_audio_engine()->get_sampler()->__main_out_R;
	for ( unsigned i = 0; i < nframes; ++i ) {
		m_pMainBuffer_L[ i ] += out_L[ i ];
		m_pMainBuffer_R[ i ] += out_R[ i ];
	}

	timeval renderTime_end = currentTime2();

	timeval ladspaTime_start = renderTime_end;
#ifdef LADSPA_SUPPORT
	// Process LADSPA FX
	if ( m_audioEngineState >= STATE_READY ) {
		for ( unsigned nFX = 0; nFX < MAX_FX; ++nFX ) {
			LadspaFX *pFX = Engine::get_instance()->get_effects()->getLadspaFX( nFX );
			if ( ( pFX ) && ( pFX->isEnabled() ) ) {
				pFX->processFX( nframes );
				float *buf_L = NULL;
				float *buf_R = NULL;
				if ( pFX->getPluginType() == LadspaFX::STEREO_FX ) {
					buf_L = pFX->m_pBuffer_L;
					buf_R = pFX->m_pBuffer_R;
				} else { // MONO FX
					buf_L = pFX->m_pBuffer_L;
					buf_R = buf_L;
				}
				for ( unsigned i = 0; i < nframes; ++i ) {
					m_pMainBuffer_L[ i ] += buf_L[ i ];
					m_pMainBuffer_R[ i ] += buf_R[ i ];
					if ( buf_L[ i ] > m_fFXPeak_L[nFX] )
						m_fFXPeak_L[nFX] = buf_L[ i ];
					if ( buf_R[ i ] > m_fFXPeak_R[nFX] )
						m_fFXPeak_R[nFX] = buf_R[ i ];
				}
			}
		}
	}
#endif
	timeval ladspaTime_end = currentTime2();

	// update master peaks
	float val_L;
	float val_R;
	if ( m_audioEngineState >= STATE_READY ) {
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

//	float fRenderTime = (renderTime_end.tv_sec - renderTime_start.tv_sec) * 1000.0 + (renderTime_end.tv_usec - renderTime_start.tv_usec) / 1000.0;
	float fLadspaTime =
		( ladspaTime_end.tv_sec - ladspaTime_start.tv_sec ) * 1000.0
		+ ( ladspaTime_end.tv_usec - ladspaTime_start.tv_usec ) / 1000.0;

	timeval finishTimeval = currentTime2();
	m_fProcessTime =
		( finishTimeval.tv_sec - startTimeval.tv_sec ) * 1000.0
		+ ( finishTimeval.tv_usec - startTimeval.tv_usec ) / 1000.0;

	m_fMaxProcessTime = 1000.0 / ( (float)pos.frame_rate / nframes );

	Engine::get_instance()->get_audio_engine()->unlock();

 	if ( m_sendPatternChange ) {
 		Engine::get_instance()->get_event_queue()->push_event( EVENT_PATTERN_CHANGED, -1 );
		m_sendPatternChange = false;
 	}

        // Increment the transport and clear out the processed sequencer notes.
        xport->processed_frames(nframes);
	m_queue.consumed(nframes);

	return 0;
}

void audioEngine_setupLadspaFX( unsigned nBufferSize )
{
	//INFOLOG( "buffersize=" + to_string(nBufferSize) );

	if ( m_pSong == NULL ) {
		//INFOLOG( "m_pSong=NULL" );
		return;
	}
	if ( nBufferSize == 0 ) {
		ERRORLOG( "nBufferSize=0" );
		return;
	}

#ifdef LADSPA_SUPPORT
	for ( unsigned nFX = 0; nFX < MAX_FX; ++nFX ) {
		LadspaFX *pFX = Engine::get_instance()->get_effects()->getLadspaFX( nFX );
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

		Engine::get_instance()->get_effects()->getLadspaFX( nFX )->connectAudioPorts(
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
	JackOutput *jao;
	jao = dynamic_cast<JackOutput*>(m_pAudioDriver);
	if ( jao ) {
		jao->makeTrackOutputs( m_pSong );
	}
#endif
}



void audioEngine_setSong( Song *newSong )
{
	WARNINGLOG( QString( "Set song: %1" ).arg( newSong->get_name() ) );

	while( m_pSong != 0 ) {
		audioEngine_removeSong();
	}

	Engine::get_instance()->get_audio_engine()->lock( RIGHT_HERE );

	m_pTransport->stop();
	audioEngine_stop( false );  // Also clears all note queues.

	// check current state
	if ( m_audioEngineState != STATE_PREPARED ) {
		ERRORLOG( "Error the audio engine is not in PREPARED state" );
	}

	Engine::get_instance()->get_event_queue()->push_event( EVENT_SELECTED_PATTERN_CHANGED, -1 );
	Engine::get_instance()->get_event_queue()->push_event( EVENT_PATTERN_CHANGED, -1 );
	Engine::get_instance()->get_event_queue()->push_event( EVENT_SELECTED_INSTRUMENT_CHANGED, -1 );

	//sleep( 1 );

	audioEngine_clearNoteQueue();

	assert( m_pSong == NULL );
	m_pSong = newSong;
	m_pTransport->set_current_song(newSong);
	m_SongSequencer.set_current_song(newSong);

	// setup LADSPA FX
	audioEngine_setupLadspaFX( m_pAudioDriver->getBufferSize() );

	audioEngine_renameJackPorts();

	// change the current audio engine state
	m_audioEngineState = STATE_READY;

	m_pTransport->locate( 0 );

	Engine::get_instance()->get_audio_engine()->unlock();

	Engine::get_instance()->get_event_queue()->push_event( EVENT_STATE, STATE_READY );
}



void audioEngine_removeSong()
{
	Engine::get_instance()->get_audio_engine()->lock( RIGHT_HERE );

	m_pTransport->stop();
	audioEngine_stop( false );

	// check current state
	if ( m_audioEngineState != STATE_READY ) {
		ERRORLOG( "Error the audio engine is not in READY state" );
		Engine::get_instance()->get_audio_engine()->unlock();
		return;
	}

	m_pSong = NULL;
	m_pTransport->set_current_song(0);
	m_SongSequencer.set_current_song(0);

	audioEngine_clearNoteQueue();

	// change the current audio engine state
	m_audioEngineState = STATE_PREPARED;
	Engine::get_instance()->get_audio_engine()->unlock();

	Engine::get_instance()->get_event_queue()->push_event( EVENT_STATE, STATE_PREPARED );
}

#if 0
//// This is dead code, but I may want to refer to it
//// when re-implementing features like lookahead
inline void audioEngine_updateNoteQueue( unsigned nFrames, const TransportPosition& pos )
{
	static int nLastTick = -1;
	bool bSendPatternChange = false;
	int nMaxTimeHumanize = 2000;
	int nLeadLagFactor = m_pAudioDriver->m_transport.m_nTickSize * 5;  // 5 ticks

	unsigned int framepos;
	if (  m_audioEngineState == STATE_PLAYING ) {
		framepos = m_pAudioDriver->m_transport.m_nFrames;
	} else {
		// use this to support realtime events when not playing
		framepos = m_nRealtimeFrames;
	}

	int tickNumber_start = 0;

	// We need to look ahead in the song for notes with negative offsets
	// from LeadLag or Humanize.  When starting from the beginning, we prime
	// the note queue with notes between 0 and nFrames plus
	// lookahead. lookahead should be equal or greater than the
	// nLeadLagFactor + nMaxTimeHumanize.
	int lookahead = nLeadLagFactor + nMaxTimeHumanize + 1;
	m_nLookaheadFrames = lookahead;
	if ( framepos == 0
	     || ( m_audioEngineState == STATE_PLAYING
		  && m_pSong->get_mode() == Song::SONG_MODE
		  && m_nSongPos == -1 ) ) {
		tickNumber_start = (int)( framepos
					  / m_pAudioDriver->m_transport.m_nTickSize );
	}
	else {
		tickNumber_start = (int)( (framepos + lookahead)
					  / m_pAudioDriver->m_transport.m_nTickSize );
	}
	int tickNumber_end = (int)( (framepos + nFrames + lookahead)
				    / m_pAudioDriver->m_transport.m_nTickSize );

	int tick = tickNumber_start;

// 	WARNINGLOG( "Lookahead: " + to_string( lookahead
//	                                        / m_pAudioDriver->m_transport.m_nTickSize ) );
	// get initial timestamp for first tick
	gettimeofday( &m_currentTickTime, NULL );
	

	while ( tick <= tickNumber_end ) {
		if ( tick == nLastTick ) {
			++tick;
			continue;
		} else {
			nLastTick = tick;
		}


		// midi events now get put into the m_songNoteQueue as well,
		// based on their timestamp
		while ( m_midiNoteQueue.size() > 0 ) {
			Note *note = m_midiNoteQueue[0];

			if ( ( int )note->get_position() <= tick ) {
				// printf ("tick=%d  pos=%d\n", tick, note->getPosition());
				m_midiNoteQueue.pop_front();
				note->get_instrument()->enqueue();
				m_songNoteQueue.push( note );
			} else {
				break;
			}
		}

		if (  m_audioEngineState != STATE_PLAYING ) {
			// only keep going if we're playing
			continue;
		}

// 		if ( m_nPatternStartTick == -1 ) { // for debugging pattern mode :s
// 			WARNINGLOG( "m_nPatternStartTick == -1; tick = "
//			             + to_string( tick ) );
// 		}


		// SONG MODE
		if ( m_pSong->get_mode() == Song::SONG_MODE ) {
			if ( m_pSong->get_pattern_group_vector()->size() == 0 ) {
				// there's no song!!
				ERRORLOG( "no patterns in song." );
				m_pAudioDriver->stop();
				return -1;
			}

			m_nSongPos = findPatternInTick( tick,
							m_pSong->is_loop_enabled(),
							&m_nPatternStartTick );
			if ( m_nSongSizeInTicks != 0 ) {
				m_nPatternTickPosition = ( tick - m_nPatternStartTick )
					                 % m_nSongSizeInTicks;
			} else {
				m_nPatternTickPosition = tick - m_nPatternStartTick;
			}

			if ( m_nPatternTickPosition == 0 ) {
				bSendPatternChange = true;
			}

//			PatternList *pPatternList =
//				 (*(m_pSong->getPatternGroupVector()))[m_nSongPos];
			if ( m_nSongPos == -1 ) {
				INFOLOG( "song pos = -1" );
				if ( m_pSong->is_loop_enabled() == true ) {
					m_nSongPos = findPatternInTick( 0,
									true,
									&m_nPatternStartTick );
				} else {
					INFOLOG( "End of Song" );
					return -1;
				}
			}
			PatternList *pPatternList =
				( *( m_pSong->get_pattern_group_vector() ) )[m_nSongPos];
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
			// per ora considero solo il primo pattern, se ce ne
			// saranno piu' di uno bisognera' prendere quello piu'
			// piccolo

			//m_nPatternTickPosition = tick % m_pCurrentPattern->getSize();
			int nPatternSize = MAX_NOTES;

			
			if ( Engine::get_instance()->get_preferences()->patternModePlaysSelected() )
			{
				m_pPlayingPatterns->clear();
				Pattern * pSelectedPattern =
					m_pSong->get_pattern_list()
					       ->get(m_nSelectedPatternNumber);
				m_pPlayingPatterns->add( pSelectedPattern );
			}


			if ( m_pPlayingPatterns->get_size() != 0 ) {
				Pattern *pFirstPattern = m_pPlayingPatterns->get( 0 );
				nPatternSize = pFirstPattern->get_length();
			}

			if ( nPatternSize == 0 ) {
				ERRORLOG( "nPatternSize == 0" );
			}

			if ( ( tick == m_nPatternStartTick + nPatternSize )
			     || ( m_nPatternStartTick == -1 ) ) {
				if ( m_pNextPatterns->get_size() > 0 ) {
					Pattern * p;
					for ( uint i = 0;
					      i < m_pNextPatterns->get_size();
					      i++ ) {
						p = m_pNextPatterns->get( i );
// 						WARNINGLOG( QString( "Got pattern # %1" )
//							     .arg( i + 1 ) );
						// if the pattern isn't playing
						// already, start it now.
						if ( ( m_pPlayingPatterns->del( p ) ) == NULL ) {
							m_pPlayingPatterns->add( p );
						}
					}
					m_pNextPatterns->clear();
					bSendPatternChange = true;
				}
				if ( m_nPatternStartTick == -1 ) {
					m_nPatternStartTick = tick - (tick % nPatternSize);
// 					WARNINGLOG( "set Pattern Start Tick to "
//						     + to_string( m_nPatternStartTick ) );
				} else {
					m_nPatternStartTick = tick;
				}
			}
			m_nPatternTickPosition = tick - m_nPatternStartTick;
			if ( m_nPatternTickPosition > nPatternSize ) {
				m_nPatternTickPosition = tick % nPatternSize;
			}
		}

		// metronome
// 		if (  ( m_nPatternStartTick == tick )
//		      || ( ( tick - m_nPatternStartTick ) % 48 == 0 ) ) {
		if ( m_nPatternTickPosition % 48 == 0 ) {
			float fPitch;
			float fVelocity;
// 			INFOLOG( "Beat: " + to_string(m_nPatternTickPosition / 48 + 1)
//				   + "@ " + to_string( tick ) );
			if ( m_nPatternTickPosition == 0 ) {
				fPitch = 3;
				fVelocity = 1.0;
				Engine::get_instance()->get_event_queue()->push_event( EVENT_METRONOME, 1 );
			} else {
				fPitch = 0;
				fVelocity = 0.8;
				Engine::get_instance()->get_event_queue()->push_event( EVENT_METRONOME, 0 );
			}
			if ( Engine::get_instance()->get_preferences()->m_bUseMetronome ) {
				m_pMetronomeInstrument->set_volume(
					Engine::get_instance()->get_preferences()->m_fMetronomeVolume
					);
				Note *pMetronomeNote = new Note( m_pMetronomeInstrument,
								 tick,
								 fVelocity,
								 0.5,
								 0.5,
								 -1,
								 fPitch
					);
				m_pMetronomeInstrument->enqueue();
				m_songNoteQueue.push( pMetronomeNote );
			}
		}

		// update the notes queue
		if ( m_pPlayingPatterns->get_size() != 0 ) {
			for ( unsigned nPat = 0 ;
			      nPat < m_pPlayingPatterns->get_size() ;
			      ++nPat ) {
				Pattern *pPattern = m_pPlayingPatterns->get( nPat );
				assert( pPattern != NULL );

				Pattern::note_map_t::iterator pos;
				for ( pos = pPattern->note_map.lower_bound( m_nPatternTickPosition ) ;
				      pos != pPattern->note_map.upper_bound( m_nPatternTickPosition ) ;
				      ++pos ) {
					Note *pNote = pos->second;
					if ( pNote ) {
						int nOffset = 0;

						// Swing
						float fSwingFactor = m_pSong->get_swing_factor();
						
						if ( ( ( m_nPatternTickPosition % 12 ) == 0 )
						     && ( ( m_nPatternTickPosition % 24 ) != 0 ) ) {
							// da l'accento al tick 4, 12, 20, 36...
							nOffset += ( int )(
								6.0
								* m_pAudioDriver->m_transport.m_nTickSize
								* fSwingFactor
								);
						}

						// Humanize - Time parameter
						if ( m_pSong->get_humanize_time_value() != 0 ) {
							nOffset += ( int )(
								getGaussian( 0.3 )
								* m_pSong->get_humanize_time_value()
								* nMaxTimeHumanize
								);
						}
						//~
						// Lead or Lag - timing parameter
						nOffset += (int) ( pNote->get_leadlag()
								   * nLeadLagFactor);
						//~

						if((tick == 0) && (nOffset < 0)) {
							nOffset = 0;
						}
						Note *pCopiedNote = new Note( pNote );
						pCopiedNote->set_position( tick );

						// humanize time
						pCopiedNote->m_nHumanizeDelay = nOffset;
						pNote->get_instrument()->enqueue();
						m_songNoteQueue.push( pCopiedNote );
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
#endif

#if 0
//// This is dead code.  Obsoleted by new transport.
//// Left here... just in case.  :-)
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
			// tengo in considerazione solo il primo pattern. I
			// pattern nel gruppo devono avere la stessa lunghezza.
			nPatternSize = pColumn->get( 0 )->get_length();
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
				// tengo in considerazione solo il primo
				// pattern. I pattern nel gruppo devono avere la
				// stessa lunghezza.
				nPatternSize = pColumn->get( 0 )->get_length();
			} else {
				nPatternSize = MAX_NOTES;
			}

			if ( ( nLoopTick >= nTotalTick )
			     && ( nLoopTick < nTotalTick + nPatternSize ) ) {
				( *pPatternStartTick ) = nTotalTick;
				return i;
			}
			nTotalTick += nPatternSize;
		}
	}

	QString err = QString( "[findPatternInTick] tick = %1. No pattern found" ).arg( QString::number(nTick) );
	ERRORLOG( err );
	return -1;
}
#endif


void audioEngine_noteOn( Note *note )
{
	m_GuiInput.note_on(note);
	delete note;  // Why are we deleting the note?
}



void audioEngine_noteOff( Note *note )
{
	if ( note == 0 ) return;
	m_GuiInput.note_off(note);
	delete note; // Why are we deleting the note?
}



// unsigned long audioEngine_getTickPosition()
// {
// 	return m_nPatternTickPosition;
// }


AudioOutput* createDriver( const QString& sDriver )
{
	INFOLOG( QString( "Driver: '%1'" ).arg( sDriver ) );
	Preferences *pPref = Engine::get_instance()->get_preferences();
	AudioOutput *pDriver = NULL;

	if ( sDriver == "Jack" ) {
#ifdef JACK_SUPPORT
		m_jack_client->open();
		#warning "Could `new JackOutput` really return NullDriver?"
		pDriver = new JackOutput( m_jack_client, audioEngine_process );
		JackOutput *jao = dynamic_cast<JackOutput*>(pDriver);
		if ( jao == 0 ) {
			delete pDriver;
			pDriver = 0;
		} else {
			jao->setConnectDefaults(
				Engine::get_instance()->get_preferences()->m_bJackConnectDefaults
				);
		}
#endif
	} else if ( sDriver == "Fake" ) {
		WARNINGLOG( "*** Using FAKE audio driver ***" );
		pDriver = new FakeDriver( audioEngine_process );
	} else {
		ERRORLOG( "Unknown driver " + sDriver );
		audioEngine_raiseError( Engine::UNKNOWN_DRIVER );
	}

	if ( pDriver  ) {
		// initialize the audio driver
		int res = pDriver->init( pPref->m_nBufferSize );
		if ( res != 0 ) {
			ERRORLOG( "Error starting audio driver [audioDriver::init()]" );
			delete pDriver;
			pDriver = NULL;
		}
	}

	return pDriver;
}


/// Start all audio drivers
void audioEngine_startAudioDrivers()
{
	Preferences *preferencesMng = Engine::get_instance()->get_preferences();

	Engine::get_instance()->get_audio_engine()->lock( RIGHT_HERE );
	QMutexLocker mx(&mutex_OutputPointer);

	INFOLOG( "[audioEngine_startAudioDrivers]" );

	// check current state
	if ( m_audioEngineState != STATE_INITIALIZED ) {
		ERRORLOG( QString( "Error the audio engine is not in INITIALIZED"
				    " state. state=%1" )
			   .arg( m_audioEngineState ) );
		Engine::get_instance()->get_audio_engine()->unlock();
		return;
	}

	if ( m_pAudioDriver ) {	// check if the audio m_pAudioDriver is still alive
		ERRORLOG( "The audio driver is still alive" );
	}
	if ( m_pMidiDriver ) {	// check if midi driver is still alive
		ERRORLOG( "The MIDI driver is still active" );
	}


	QString sAudioDriver = preferencesMng->m_sAudioDriver;
//	sAudioDriver = "Auto";
	if ( sAudioDriver == "Auto" ) {
		if ( ( m_pAudioDriver = createDriver( "Jack" ) ) == NULL ) {
			audioEngine_raiseError( Engine::ERROR_STARTING_DRIVER );
			ERRORLOG( "Error starting audio driver" );
			ERRORLOG( "Using the NULL output audio driver" );

			// use the NULL output driver
			m_pAudioDriver = new NullDriver( audioEngine_process );
			m_pAudioDriver->init( 0 );
		}
	} else {
		m_pAudioDriver = createDriver( sAudioDriver );
		if ( m_pAudioDriver == NULL ) {
			audioEngine_raiseError( Engine::ERROR_STARTING_DRIVER );
			ERRORLOG( "Error starting audio driver" );
			ERRORLOG( "Using the NULL output audio driver" );
			
			// use the NULL output driver
			m_pAudioDriver = new NullDriver( audioEngine_process );
			m_pAudioDriver->init( 0 );
		}
	}
	
	if ( preferencesMng->m_sMidiDriver == "JackMidi" ) {
#ifdef JACK_SUPPORT
		m_jack_client->open();
		m_pMidiDriver = new JackMidiDriver(m_jack_client);
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


	if ( m_audioEngineState == STATE_PREPARED ) {
		Engine::get_instance()->get_event_queue()->push_event( EVENT_STATE, STATE_PREPARED );
	} else if ( m_audioEngineState == STATE_READY ) {
		Engine::get_instance()->get_event_queue()->push_event( EVENT_STATE, STATE_READY );
	}

	// Unlocking earlier might execute the jack process() callback before we
	// are fully initialized.
	mx.unlock();
	Engine::get_instance()->get_audio_engine()->unlock();

#ifdef JACK_SUPPORT
	if( m_jack_client->ref() ) {
		m_jack_client->activate();
	}
#endif

	if ( m_pAudioDriver ) {
		int res = m_pAudioDriver->connect();
		if ( res != 0 ) {
			audioEngine_raiseError( Engine::ERROR_STARTING_DRIVER );
			ERRORLOG( "Error starting audio driver [audioDriver::connect()]" );
			ERRORLOG( "Using the NULL output audio driver" );

			mx.relock();
			delete m_pAudioDriver;
			m_pAudioDriver = new NullDriver( audioEngine_process );
			mx.unlock();
			m_pAudioDriver->init( 0 );
			m_pAudioDriver->connect();
		}

                #warning "Caching output port buffer pointers is deprecated in " \
                    "JACK.  JACK 2.0 will require that output ports get a new " \
                    "buffer pointer for every process() cycle."
		if ( ( m_pMainBuffer_L = m_pAudioDriver->getOut_L() ) == NULL ) {
			ERRORLOG( "m_pMainBuffer_L == NULL" );
		}
		if ( ( m_pMainBuffer_R = m_pAudioDriver->getOut_R() ) == NULL ) {
			ERRORLOG( "m_pMainBuffer_R == NULL" );
		}

#ifdef JACK_SUPPORT
		audioEngine_renameJackPorts();
#endif

		audioEngine_setupLadspaFX( m_pAudioDriver->getBufferSize() );
	}


}



/// Stop all audio drivers
void audioEngine_stopAudioDrivers()
{
	INFOLOG( "[audioEngine_stopAudioDrivers]" );

	Engine::get_instance()->get_transport()->stop();

	if ( ( m_audioEngineState != STATE_PREPARED )
	     && ( m_audioEngineState != STATE_READY ) ) {
		ERRORLOG( QString( "Error: the audio engine is not in PREPARED"
				    " or READY state. state=%1" )
			   .arg( m_audioEngineState ) );
		return;
	}

	// change the current audio engine state
	m_audioEngineState = STATE_INITIALIZED;
	Engine::get_instance()->get_event_queue()->push_event( EVENT_STATE, STATE_INITIALIZED );

	Engine::get_instance()->get_audio_engine()->lock( RIGHT_HERE );

	// delete MIDI driver
	if ( m_pMidiDriver ) {
		m_pMidiDriver->close();
		delete m_pMidiDriver;
		m_pMidiDriver = NULL;
	}

	// delete audio driver
	if ( m_pAudioDriver ) {
		m_pAudioDriver->disconnect();
		QMutexLocker mx( &mutex_OutputPointer );
		delete m_pAudioDriver;
		m_pAudioDriver = NULL;
		mx.unlock();
	}

#ifdef JACK_SUPPORT
	m_jack_client->close();
#endif

	Engine::get_instance()->get_audio_engine()->unlock();
}



/// Restart all audio and midi drivers
void audioEngine_restartAudioDrivers()
{
	audioEngine_stopAudioDrivers();
	audioEngine_startAudioDrivers();
}






//----------------------------------------------------------------------------
//
// Implementation of Engine class
//
//----------------------------------------------------------------------------

/// static reference of Engine class (Singleton)
Engine* Engine::__instance = NULL;




Engine::Engine(Preferences* prefs)
{
	assert(prefs);
	m_preferences = prefs;

	INFOLOG( "[Engine]" );

	__instance = this;
	engineInstance = this;
	m_pTransport = new H2Transport;

	audioEngine_init();
	audioEngine_startAudioDrivers();
}



Engine::~Engine()
{
	INFOLOG( "[~Engine]" );
	m_pTransport->stop();
	removeSong();
	audioEngine_stopAudioDrivers();
	audioEngine_destroy();
	__kill_instruments();
	delete m_action_manager;
	delete m_event_queue;
	delete m_pTransport;
	delete m_preferences;
	__instance = 0;
}



void Engine::create_instance(Preferences *prefs)
{
	assert(prefs);
	m_preferences = prefs;
	// Create all the other instances that we need
	// ....and in the right order
	Logger::create_instance();
	m_event_queue = new EventQueue();
	m_action_manager = new ActionManager();

	if( __instance == 0 ) {
		__instance = new Engine(prefs);
	}

	// See audioEngine_init() for:
	// AudioEngine, Effects, Playlist
}

Preferences* Engine::get_preferences()
{
	return m_preferences;
}

AudioEngine* Engine::get_audio_engine()
{
	return m_audio_engine;
}

Transport* Engine::get_transport()
{
	return static_cast<Transport*>(m_pTransport);
}

ActionManager* Engine::get_action_manager()
{
	return m_action_manager;
}

EventQueue* Engine::get_event_queue()
{
	return m_event_queue;
}

Playlist* Engine::get_playlist()
{
	return m_playlist;
}

#ifdef LADSPA_SUPPORT
Effects* Engine::get_effects()
{
	return m_effects;
}
#endif

/// Start the internal sequencer
void Engine::sequencer_play()
{
	m_pTransport->start();
}

/// Stop the internal sequencer
void Engine::sequencer_stop()
{
	m_pTransport->stop();
}



void Engine::setSong( Song *pSong )
{
	while( m_pSong != 0 ) {
		removeSong();
	}
	audioEngine_setSong( pSong );
}



void Engine::removeSong()
{
	audioEngine_removeSong();
}



Song* Engine::getSong()
{
	return m_pSong;
}



void Engine::midi_noteOn( Note *note )
{
	audioEngine_noteOn( note );
}



void Engine::midi_noteOff( Note *note )
{
	audioEngine_noteOff( note );
}

void Engine::addRealtimeNote( int instrument,
				float velocity,
				float pan_L,
				float pan_R,
				float /* pitch */,
				bool /* forcePlay */,
				bool use_frame,
				uint32_t frame )
{
	Preferences *pref = Engine::get_instance()->get_preferences();
	Instrument* i = getSong()->get_instrument_list()->get(instrument);
	Note note( i,
		   velocity,
		   pan_L,
		   pan_R,
		   -1
		);
	m_GuiInput.note_on(&note, pref->getQuantizeEvents());
	#warning "JACK MIDI note timing is getting lost here"
}



float Engine::getMasterPeak_L()
{
	return m_fMasterPeak_L;
}



float Engine::getMasterPeak_R()
{
	return m_fMasterPeak_R;
}



unsigned long Engine::getTickPosition()
{
	TransportPosition pos;
	m_pTransport->get_position(&pos);
	return pos.tick + (pos.beat-1) * pos.ticks_per_beat;
}

PatternList* Engine::getCurrentPatternList()
{
	TransportPosition pos;
	m_pTransport->get_position(&pos);
	if( pos.bar <= m_pSong->get_pattern_group_vector()->size() ) {
		return m_pSong->get_pattern_group_vector()->at(pos.bar-1);
	} else {
		return 0;
	}
}

PatternList * Engine::getNextPatterns()
{
	static PatternList the_nothing;
	TransportPosition pos;
	m_pTransport->get_position(&pos);
	size_t p_sz = m_pSong->get_pattern_group_vector()->size();
	if( pos.bar < p_sz ) {
		return m_pSong->get_pattern_group_vector()->at(pos.bar);
	} else {
		if( m_pSong->is_loop_enabled() && p_sz ) {
			return m_pSong->get_pattern_group_vector()->at(0);
		} else  {
			return &the_nothing;
		}
	}
}

/// Set the next pattern (Pattern mode only)
void Engine::sequencer_setNextPattern( int pos, bool /*appendPattern*/, bool /*deletePattern*/ )
{
	m_pSong->set_next_pattern(pos);
}



int Engine::getPatternPos()
{
	TransportPosition pos;
	m_pTransport->get_position(&pos);
	return pos.bar-1;
}



void Engine::restartDrivers()
{
	audioEngine_restartAudioDrivers();
}



/// Export a song to a wav file, returns the elapsed time in mSec
void Engine::startExportSong( const QString& filename )
{
	m_pTransport->stop();
	Preferences *pPref = Engine::get_instance()->get_preferences();

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

	Engine::get_instance()->get_audio_engine()->get_sampler()->stop_playing_notes();

	// reset
	m_pTransport->locate( 0 );

	int res = m_pAudioDriver->init( pPref->m_nBufferSize );
	if ( res != 0 ) {
		ERRORLOG( "Error starting disk writer driver "
			   "[DiskWriterDriver::init()]" );
	}

	m_pMainBuffer_L = m_pAudioDriver->getOut_L();
	m_pMainBuffer_R = m_pAudioDriver->getOut_R();

	audioEngine_setupLadspaFX( m_pAudioDriver->getBufferSize() );

	m_pTransport->locate(0);

	res = m_pAudioDriver->connect();
	if ( res != 0 ) {
		ERRORLOG( "Error starting disk writer driver "
			   "[DiskWriterDriver::connect()]" );
	}
}



void Engine::stopExportSong()
{
	if ( ! dynamic_cast<DiskWriterDriver*>(m_pAudioDriver) ) {
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

	audioEngine_startAudioDrivers();

}



/// Used to display audio driver info
AudioOutput* Engine::get_audio_output()
{
	return m_pAudioDriver;
}



/// Used to display midi driver info
MidiInput* Engine::get_midi_input()
{
	return m_pMidiDriver;
}



void Engine::setMasterPeak_L( float value )
{
	m_fMasterPeak_L = value;
}



void Engine::setMasterPeak_R( float value )
{
	m_fMasterPeak_R = value;
}



int Engine::getState()
{
	return m_audioEngineState;
}

float Engine::getProcessTime()
{
	return m_fProcessTime;
}



float Engine::getMaxProcessTime()
{
	return m_fMaxProcessTime;
}



int Engine::loadDrumkit( Drumkit *drumkitInfo )
{
	int old_ae_state = m_audioEngineState;
	if( m_audioEngineState >= STATE_READY ) {
		m_audioEngineState = STATE_PREPARED;
	}

	INFOLOG( drumkitInfo->getName() );
	m_currentDrumkit = drumkitInfo->getName();
	LocalFileMng fileMng;
	QString sDrumkitPath = fileMng.getDrumkitDirectory( drumkitInfo->getName() );


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
		
	wolke: info!
		this has moved to the end of this function
		because we get lost objects in memory
		now: 
		1. the new drumkit will loaded
		2. all not used instruments will complete deleted 
	
	old funktion:
	while ( pDrumkitInstrList->get_size() < songInstrList->get_size() )
	{
		songInstrList->del(songInstrList->get_size() - 1);
	}
	*/
	
	//needed for the new delete function
	int instrumentDiff =  songInstrList->get_size() - pDrumkitInstrList->get_size();

	for ( unsigned nInstr = 0; nInstr < pDrumkitInstrList->get_size(); ++nInstr ) {
		Instrument *pInstr = NULL;
		if ( nInstr < songInstrList->get_size() ) {
			//instrument exists already
			pInstr = songInstrList->get( nInstr );
			assert( pInstr );
		} else {
			pInstr = Instrument::create_empty();
			// The instrument isn't playing yet; no need for locking
			// :-) - Jakob Lund.  Engine::get_instance()->get_audio_engine()->lock(
			// "Engine::loadDrumkit" );
			songInstrList->add( pInstr );
			// Engine::get_instance()->get_audio_engine()->unlock();
		}

		Instrument *pNewInstr = pDrumkitInstrList->get( nInstr );
		assert( pNewInstr );
		INFOLOG( QString( "Loading instrument (%1 of %2) [%3]" )
			  .arg( nInstr )
			  .arg( pDrumkitInstrList->get_size() )
			  .arg( pNewInstr->get_name() ) );
		
		// creo i nuovi layer in base al nuovo strumento
		// Moved code from here right into the Instrument class - Jakob Lund.
		pInstr->load_from_placeholder( pNewInstr );
	}


//wolke: new delete funktion
	if ( instrumentDiff >=0	){
		for ( int i = 0; i < instrumentDiff ; i++ ){
			removeInstrument(
				m_pSong->get_instrument_list()->get_size() - 1,
				true
				);
		}
	}

	#ifdef JACK_SUPPORT
	Engine::get_instance()->get_audio_engine()->lock( RIGHT_HERE );
		renameJackPorts();
	Engine::get_instance()->get_audio_engine()->unlock();
	#endif

	m_audioEngineState = old_ae_state;

	return 0;	//ok
}


//this is also a new function and will used from the new delete function in
//Engine::loadDrumkit to delete the instruments by number
void Engine::removeInstrument( int instrumentnumber, bool conditional )
{
	Instrument *pInstr = m_pSong->get_instrument_list()->get( instrumentnumber );


	PatternList* pPatternList = getSong()->get_pattern_list();
	
	if ( conditional ) {
	// new! this check if a pattern has an active note if there is an note
	//inside the pattern the intrument would not be deleted
		for ( int nPattern = 0 ;
		      nPattern < (int)pPatternList->get_size() ;
		      ++nPattern ) {
			if( pPatternList
			    ->get( nPattern )
			    ->references_instrument( pInstr ) ) {
				return;
			}
		}
	} else {
		getSong()->purge_instrument( pInstr );
	}

	Song *pSong = getSong();
	InstrumentList* pList = pSong->get_instrument_list();
	if(pList->get_size()==1){
		Engine::get_instance()->get_audio_engine()->lock( RIGHT_HERE );
		Instrument* pInstr = pList->get( 0 );
		pInstr->set_name( (QString( "Instrument 1" )) );
		// remove all layers
		for ( int nLayer = 0; nLayer < MAX_LAYERS; nLayer++ ) {
			InstrumentLayer* pLayer = pInstr->get_layer( nLayer );
			delete pLayer;
			pInstr->set_layer( NULL, nLayer );
		}		
	Engine::get_instance()->get_audio_engine()->unlock();
	Engine::get_instance()->get_event_queue()->push_event( EVENT_SELECTED_INSTRUMENT_CHANGED, -1 );
	INFOLOG("clear last instrument to empty instrument 1 instead delete the last instrument");
	return;
	}

	// if the instrument was the last on the instruments list, select the
	// next-last
	if ( instrumentnumber
	     >= (int)getSong()->get_instrument_list()->get_size() - 1 ) {
		Engine::get_instance()
			->setSelectedInstrumentNumber(
				std::max(0, instrumentnumber - 1)
				);
	}
	// delete the instrument from the instruments list
	Engine::get_instance()->get_audio_engine()->lock( RIGHT_HERE );
	getSong()->get_instrument_list()->del( instrumentnumber );
	getSong()->set_modified(true);
	Engine::get_instance()->get_audio_engine()->unlock();
	
	// At this point the instrument has been removed from both the
	// instrument list and every pattern in the song.  Hence there's no way
	// (NOTE) to play on that instrument, and once all notes have stopped
	// playing it will be save to delete.
	// the ugly name is just for debugging...
	QString xxx_name = QString( "XXX_%1" ) . arg( pInstr->get_name() );
	pInstr->set_name( xxx_name );
	__instrument_death_row.push_back( pInstr );
	__kill_instruments(); // checks if there are still notes.
	
	// this will force a GUI update.
	Engine::get_instance()->get_event_queue()->push_event( EVENT_SELECTED_INSTRUMENT_CHANGED, -1 );
}


void Engine::raiseError( unsigned nErrorCode )
{
	audioEngine_raiseError( nErrorCode );
}


unsigned long Engine::getRealtimeFrames()
{
	return m_nFreeRollingFrameCounter;
}

/**
 * Get the ticks for pattern at pattern pos
 * @a int pos -- position in song
 * @return -1 if pos > number of patterns in the song, tick no. > 0 otherwise
 * The driver should be LOCKED when calling this!!
 */
long Engine::getTickForPosition( int pos )
{
	int nPatternGroups = m_pSong->get_pattern_group_vector()->size();
	if( nPatternGroups == 0 ) return -1;	

	if ( pos >= nPatternGroups ) {
		if ( m_pSong->is_loop_enabled() ) {
			pos = pos % nPatternGroups;
		} else {
			WARNINGLOG( QString( "patternPos > nPatternGroups. pos:"
					      " %1, nPatternGroups: %2")
				     .arg( pos )
				     .arg(  nPatternGroups ) );
			return -1;
		}
	}

	Song::pattern_group_t *pColumns = m_pSong->get_pattern_group_vector();
	long totalTick = 0;
	int nPatternSize;
	Pattern *pPattern = NULL;
	for ( int i = 0; i < pos; ++i ) {
		PatternList *pColumn = ( *pColumns )[ i ];
		// prendo solo il primo. I pattern nel gruppo devono avere la
		// stessa lunghezza
		pPattern = pColumn->get( 0 );
		if ( pPattern ) {
			nPatternSize = pPattern->get_length();
		} else {
			nPatternSize = MAX_NOTES;
		}

		totalTick += nPatternSize;
	}
	return totalTick;
}

/// Set the position in the song
void Engine::setPatternPos( int pos )
{
	m_pTransport->locate(pos+1, 1, 0);
}

void Engine::getLadspaFXPeak( int nFX, float *fL, float *fR )
{
#ifdef LADSPA_SUPPORT
	( *fL ) = m_fFXPeak_L[nFX];
	( *fR ) = m_fFXPeak_R[nFX];
#else
	( *fL ) = 0;
	( *fR ) = 0;
#endif
}



void Engine::setLadspaFXPeak( int nFX, float fL, float fR )
{
#ifdef LADSPA_SUPPORT
	m_fFXPeak_L[nFX] = fL;
	m_fFXPeak_R[nFX] = fR;
#endif
}


void Engine::onTapTempoAccelEvent()
{
	m_BeatCounter.onTapTempoAccelEvent();
}

void Engine::setTapTempo( float fInterval )
{
	m_BeatCounter.setTapTempo(fInterval);
}


void Engine::setBPM( float fBPM )
{
	if( (fBPM < 500.0) && (fBPM > 20.0) ) {
		m_pSong->set_bpm(fBPM);
	}
}



void Engine::restartLadspaFX()
{
	if ( m_pAudioDriver ) {
		Engine::get_instance()->get_audio_engine()->lock( RIGHT_HERE );
		audioEngine_setupLadspaFX( m_pAudioDriver->getBufferSize() );
		Engine::get_instance()->get_audio_engine()->unlock();
	} else {
		ERRORLOG( "m_pAudioDriver = NULL" );
	}
}



int Engine::getSelectedPatternNumber()
{
	return m_nSelectedPatternNumber;
}



void Engine::setSelectedPatternNumber( int nPat )
{
	// FIXME: controllare se e' valido..
	if ( nPat == m_nSelectedPatternNumber )	return;
	
	
	if ( Engine::get_instance()->get_preferences()->patternModePlaysSelected() ) {
		Engine::get_instance()->get_audio_engine()->lock( RIGHT_HERE );
	
		m_nSelectedPatternNumber = nPat;
		Engine::get_instance()->get_audio_engine()->unlock();
	} else {
		m_nSelectedPatternNumber = nPat;
	}

	Engine::get_instance()->get_event_queue()->push_event( EVENT_SELECTED_PATTERN_CHANGED, -1 );
}



int Engine::getSelectedInstrumentNumber()
{
	return m_nSelectedInstrumentNumber;
}



void Engine::setSelectedInstrumentNumber( int nInstrument )
{
	if ( m_nSelectedInstrumentNumber == nInstrument )	return;

	m_nSelectedInstrumentNumber = nInstrument;
	Engine::get_instance()->get_event_queue()->push_event( EVENT_SELECTED_INSTRUMENT_CHANGED, -1 );
}


#ifdef JACK_SUPPORT
void Engine::renameJackPorts()
{
	if( Engine::get_instance()->get_preferences()->m_bJackTrackOuts == true ){
		audioEngine_renameJackPorts();
	}
}
#endif


///BeatCounter

void Engine::setbeatsToCount( int beatstocount)
{
	m_BeatCounter.setBeatsToCount(beatstocount);
}


int Engine::getbeatsToCount()
{
	return m_BeatCounter.getBeatsToCount();
}


void Engine::setNoteLength( float notelength)
{
	m_BeatCounter.setNoteLength(notelength);
}



float Engine::getNoteLength()
{
	return m_BeatCounter.getNoteLength();
}



int Engine::getBcStatus()
{
	return m_BeatCounter.status();
}


void Engine::setBcOffsetAdjust()
{
	m_BeatCounter.setOffsetAdjust();
}


void Engine::handleBeatCounter()
{
	m_BeatCounter.trigger();
}
//~ beatcounter

// jack transport master

bool Engine::setJackTimeMaster(bool if_none_already)
{
	return m_pTransport->setJackTimeMaster(m_jack_client, if_none_already);
}

void Engine::clearJackTimeMaster()
{
	m_pTransport->clearJackTimeMaster();
}

bool Engine::getJackTimeMaster()
{
	return m_pTransport->getJackTimeMaster();
}

//~ jack transport master

/**
 * Toggles between SINGLE-PATTERN pattern mode, and STACKED pattern
 * mode.  In stacked pattern mode, more than one pattern may be
 * playing at once.  Also called "Live" mode.
 */
void Engine::togglePlaysSelected()
{
	Preferences * P = Engine::get_instance()->get_preferences();
	bool isPlaysSelected = P->patternModePlaysSelected();

	// NEED TO IMPLEMENT!!
	assert(false);

	P->setPatternModePlaysSelected( !isPlaysSelected );
	
}

#ifdef JACK_SUPPORT
int jackMidiFallbackProcess(jack_nframes_t nframes, void* /*arg*/)
{
    JackMidiDriver* instance =
	dynamic_cast<JackMidiDriver*>(m_pMidiDriver);
    return instance->processNonAudio(nframes);
}
#endif

void Engine::__kill_instruments()
{
	int c = 0;
	Instrument * pInstr = NULL;
	while ( __instrument_death_row.size()
		&& __instrument_death_row.front()->is_queued() == 0 ) {
		pInstr = __instrument_death_row.front();
		__instrument_death_row.pop_front();
		INFOLOG( QString( "Deleting unused instrument (%1). "
				  "%2 unused remain." )
			. arg( pInstr->get_name() )
			. arg( __instrument_death_row.size() ) );
		delete pInstr;
		c++;
	}
	if ( __instrument_death_row.size() ) {
		pInstr = __instrument_death_row.front();
		INFOLOG( QString( "Instrument %1 still has %2 active notes. "
				  "Delaying 'delete instrument' operation." )
			. arg( pInstr->get_name() )
			. arg( pInstr->is_queued() ) );
	}
}



void Engine::__panic()
{
	sequencer_stop();	
	Engine::get_instance()->get_audio_engine()->get_sampler()->stop_playing_notes();
}


};
