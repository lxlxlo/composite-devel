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
#include <Tritium/SeqEvent.hpp>
#include <Tritium/SeqScript.hpp>
#include <Tritium/SeqScriptIterator.hpp>
#include <Tritium/memory.hpp>
#include "transport/H2Transport.hpp"
#include "BeatCounter.hpp"
#include "SongSequencer.hpp"

#include "IO/FakeDriver.hpp"
#include "IO/DiskWriterDriver.hpp"
#include "IO/JackMidiDriver.hpp"
#include "IO/JackClient.hpp"

#include "EnginePrivate.hpp"

namespace Tritium
{

// GLOBALS


// PROTOTYPES
    inline timeval currentTime2()
    {
        struct timeval now;
        gettimeofday( &now, NULL );
        return now;
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

    static int engine_process_callback(uint32_t frames, void* arg)
    {
        return static_cast<EnginePrivate*>(arg)->audioEngine_process(frames);
    }


    void EnginePrivate::audioEngine_raiseError( unsigned nErrorCode )
    {
        m_engine->get_event_queue()->push_event( EVENT_ERROR, nErrorCode );
    }


/*
  void updateTickSize()
  {
  float sampleRate = ( float )m_pAudioDriver->getSampleRate();
  m_pAudioDriver->m_transport.m_nTickSize =
  ( sampleRate * 60.0 /  m_pSong->__bpm / m_pSong->__resolution );
  }
*/

    void EnginePrivate::audioEngine_init()
    {
        INFOLOG( "*** Engine audio engine init ***" );

        // check current state
        if ( m_audioEngineState != Engine::StateUninitialized ) {
            ERRORLOG( "Error the audio engine is not in UNINITIALIZED state" );
            m_engine->unlock();
            return;
        }

        m_nFreeRollingFrameCounter = 0;
        m_nSelectedPatternNumber = 0;
        m_nSelectedInstrumentNumber = 0;

        m_pMainBuffer_L = NULL;
        m_pMainBuffer_R = NULL;

        srand( time( NULL ) );

        // Create metronome instrument
        QString sMetronomeFilename = QString( "%1/click.wav" )
            .arg( DataPath::get_data_path() );
        m_pMetronomeInstrument.reset(
            new Instrument( sMetronomeFilename, "metronome", new ADSR() )
	    );
        m_pMetronomeInstrument->set_layer(
            new InstrumentLayer( Sample::load( sMetronomeFilename ) ),
            0
            );

        // Change the current audio engine state
        m_audioEngineState = Engine::StateInitialized;

#ifdef JACK_SUPPORT
        m_jack_client.reset( new JackClient(m_engine, false) );
#endif
#ifdef LADSPA_SUPPORT
        m_effects.reset( new Effects(m_engine) );
#endif
	m_mixer.reset( new MixerImpl(MAX_BUFFER_SIZE, m_effects, 4) );
        m_sampler.reset( new Sampler(boost::dynamic_pointer_cast<AudioPortManager>(m_mixer)) );
	m_sampler->set_max_note_limit( m_engine->get_preferences()->m_nMaxNotes );
        m_playlist.reset( new Playlist(m_engine) );

        m_pSong = Song::get_default_song(m_engine);

        m_engine->get_event_queue()->push_event( EVENT_STATE, Engine::StateInitialized );

    }



    void EnginePrivate::audioEngine_destroy()
    {
        // check current state
        if ( m_audioEngineState != Engine::StateInitialized ) {
            ERRORLOG( "Error the audio engine is not in INITIALIZED state" );
            return;
        }
        m_engine->get_sampler()->panic();

        m_engine->lock( RIGHT_HERE );
        INFOLOG( "*** Engine audio engine shutdown ***" );

        audioEngine_clearNoteQueue();

        // change the current audio engine state
        m_audioEngineState = Engine::StateUninitialized;
        m_engine->get_event_queue()->push_event( EVENT_STATE, Engine::StateUninitialized );

        m_pMetronomeInstrument.reset();

        m_engine->unlock();
#ifdef LADSPA_SUPPORT
        m_effects.reset();
#endif
        m_sampler.reset();
#ifdef JACK_SUPPORT
        m_jack_client.reset();
#endif
    }





    /// Start playing
    /// return 0 = OK
    /// return -1 = NULL Audio Driver
    /// return -2 = Driver connect() error
    int EnginePrivate::audioEngine_start( bool bLockEngine, unsigned nTotalFrames )
    {
        if ( bLockEngine ) {
            m_engine->lock( RIGHT_HERE );
        }

        INFOLOG( "[EnginePrivate::audioEngine_start]" );

        // check current state
        if ( m_audioEngineState != Engine::StateReady ) {
            ERRORLOG( "Error the audio engine is not in READY state" );
            if ( bLockEngine ) {
                m_engine->unlock();
            }
            return 0;   // FIXME!!
        }

        m_fMasterPeak_L = 0.0f;
        m_fMasterPeak_R = 0.0f;
        /*
          m_pAudioDriver->m_transport.m_nFrames = nTotalFrames; // reset total frames
          m_nSongPos = -1;
          m_nPatternStartTick = -1;
          m_nPatternTickPosition = 0;

          // prepare the tickSize for this song
          updateTickSize();

        */
        m_pTransport->start();

        if ( bLockEngine ) {
            m_engine->unlock();
        }
        return 0; // per ora restituisco sempre OK
    }



    /// Stop the audio engine
    void EnginePrivate::audioEngine_stop( bool bLockEngine )
    {
        if ( bLockEngine ) {
            m_engine->lock( RIGHT_HERE );
        }
        INFOLOG( "[EnginePrivate::audioEngine_stop]" );

        // check current state
        if ( m_audioEngineState != Engine::StateReady ) {
            if ( bLockEngine ) {
                m_engine->unlock();
            }
            return;
        }

        // change the current audio engine state
        /*
          m_audioEngineState = Engine::StateReady;
        */
        m_pTransport->stop();
        m_engine->get_event_queue()->push_event( EVENT_STATE, Engine::StateReady );

        m_fMasterPeak_L = 0.0f;
        m_fMasterPeak_R = 0.0f;

        audioEngine_clearNoteQueue();

        if ( bLockEngine ) {
            m_engine->unlock();
        }
    }

    void EnginePrivate::audioEngine_clearNoteQueue()
    {
        m_queue.clear();
        m_GuiInput.clear();
        m_engine->get_sampler()->panic();
    }

/// Clear all audio buffers
    inline void EnginePrivate::audioEngine_process_clearAudioBuffers( uint32_t nFrames )
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
        JackOutput* jo = dynamic_cast<JackOutput*>(m_pAudioDriver.get());
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
    }

/// Main audio processing function. Called by audio drivers.
    int EnginePrivate::audioEngine_process( uint32_t nframes )
    {
        timeval startTimeval = currentTime2();
        m_nFreeRollingFrameCounter += nframes;

	m_mixer->pre_process(nframes);
        audioEngine_process_clearAudioBuffers( nframes );

        if( m_audioEngineState < Engine::StateReady) {
            return 0;
        }

        // Hook for MIDI in-process callbacks.  It calls its own locks
        // on the audioengine
        if (m_pMidiDriver) m_pMidiDriver->processAudio(nframes);

        m_engine->lock( RIGHT_HERE );

        if( m_audioEngineState < Engine::StateReady) {
            m_engine->unlock();
            return 0;
        }

        T<Transport>::shared_ptr xport = m_engine->get_transport();
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
        T<Sampler>::shared_ptr pSampler = m_engine->get_sampler();
        pSampler->process( m_queue.begin_const(),
                           m_queue.end_const(nframes),
                           pos,
                           nframes
            );

        timeval renderTime_end = currentTime2();

        timeval ladspaTime_start = renderTime_end;
	m_mixer->mix_send_return(nframes);
        timeval ladspaTime_end = currentTime2();

	m_mixer->mix_down(nframes, m_pMainBuffer_L, m_pMainBuffer_R,
			  &m_fMasterPeak_L, &m_fMasterPeak_R);

//      float fRenderTime = (renderTime_end.tv_sec - renderTime_start.tv_sec) * 1000.0 + (renderTime_end.tv_usec - renderTime_start.tv_usec) / 1000.0;
        float fLadspaTime =
            ( ladspaTime_end.tv_sec - ladspaTime_start.tv_sec ) * 1000.0
            + ( ladspaTime_end.tv_usec - ladspaTime_start.tv_usec ) / 1000.0;

        timeval finishTimeval = currentTime2();
        m_fProcessTime =
            ( finishTimeval.tv_sec - startTimeval.tv_sec ) * 1000.0
            + ( finishTimeval.tv_usec - startTimeval.tv_usec ) / 1000.0;

        m_fMaxProcessTime = 1000.0 / ( (float)pos.frame_rate / nframes );

        m_engine->unlock();

        if ( m_sendPatternChange ) {
            m_engine->get_event_queue()->push_event( EVENT_PATTERN_CHANGED, -1 );
            m_sendPatternChange = false;
        }

        // Increment the transport and clear out the processed sequencer notes.
        xport->processed_frames(nframes);
        m_queue.consumed(nframes);

        return 0;
    }

    void EnginePrivate::audioEngine_setupLadspaFX( unsigned nBufferSize )
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
            T<LadspaFX>::shared_ptr pFX = m_engine->get_effects()->getLadspaFX( nFX );
            if ( pFX == NULL ) {
                return;
            }

            pFX->deactivate();

            m_engine->get_effects()->getLadspaFX( nFX )->connectAudioPorts(
                pFX->m_pBuffer_L,
                pFX->m_pBuffer_R,
                pFX->m_pBuffer_L,
                pFX->m_pBuffer_R
                );
            pFX->activate();
        }
#endif
    }



    void EnginePrivate::audioEngine_renameJackPorts()
    {
#ifdef JACK_SUPPORT
        // renames jack ports
        if ( m_pSong == NULL ) {
            return;
        }
        JackOutput *jao;
        jao = dynamic_cast<JackOutput*>(m_pAudioDriver.get());
        if ( jao ) {
            jao->makeTrackOutputs( m_pSong );
        }
#endif
    }



    void EnginePrivate::audioEngine_setSong( T<Song>::shared_ptr newSong )
    {
        WARNINGLOG( QString( "Set song: %1" ).arg( newSong->get_name() ) );

        while( m_pSong != 0 ) {
            audioEngine_removeSong();
        }

        m_engine->lock( RIGHT_HERE );

        m_pTransport->stop();
        audioEngine_stop( false );  // Also clears all note queues.

        // check current state
        if ( m_audioEngineState != Engine::StatePrepared ) {
            ERRORLOG( "Error the audio engine is not in PREPARED state" );
        }

        m_engine->get_event_queue()->push_event( EVENT_SELECTED_PATTERN_CHANGED, -1 );
        m_engine->get_event_queue()->push_event( EVENT_PATTERN_CHANGED, -1 );
        m_engine->get_event_queue()->push_event( EVENT_SELECTED_INSTRUMENT_CHANGED, -1 );

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
        m_audioEngineState = Engine::StateReady;

        m_pTransport->locate( 0 );

        m_engine->unlock();

        m_engine->get_event_queue()->push_event( EVENT_STATE, Engine::StateReady );
    }



    void EnginePrivate::audioEngine_removeSong()
    {
        m_engine->lock( RIGHT_HERE );

        m_pTransport->stop();
        audioEngine_stop( false );

        // check current state
        if ( m_audioEngineState != Engine::StateReady ) {
            INFOLOG( "Error the audio engine is not in READY state" );
            m_engine->unlock();
            return;
        }

        m_pSong.reset();
        m_pTransport->set_current_song( m_pSong );
        m_SongSequencer.set_current_song( m_pSong );

        audioEngine_clearNoteQueue();

        // change the current audio engine state
        m_audioEngineState = Engine::StatePrepared;
        m_engine->unlock();

        m_engine->get_event_queue()->push_event( EVENT_STATE, Engine::StatePrepared );
    }

    void EnginePrivate::audioEngine_noteOn( Note *note )
    {
        m_GuiInput.note_on(note);
        delete note;  // Why are we deleting the note?
    }



    void EnginePrivate::audioEngine_noteOff( Note *note )
    {
        if ( note == 0 ) return;
        m_GuiInput.note_off(note);
        delete note; // Why are we deleting the note?
    }

    T<AudioOutput>::shared_ptr EnginePrivate::createDriver( const QString& sDriver )
    {
        INFOLOG( QString( "Driver: '%1'" ).arg( sDriver ) );
        T<Preferences>::shared_ptr pPref = m_engine->get_preferences();
	T<AudioOutput>::shared_ptr pDriver;

        if ( sDriver == "Jack" ) {
#ifdef JACK_SUPPORT
            m_jack_client->open();
#warning "Could `new JackOutput` really return NullDriver?"
            pDriver.reset( new JackOutput( m_engine, m_jack_client, engine_process_callback, this ) );
            JackOutput *jao = dynamic_cast<JackOutput*>(pDriver.get());
            if ( jao == 0 ) {
		pDriver.reset();
            } else {
                jao->setConnectDefaults(
                    m_engine->get_preferences()->m_bJackConnectDefaults
                    );
            }
#endif
        } else if ( sDriver == "Fake" ) {
            WARNINGLOG( "*** Using FAKE audio driver ***" );
            pDriver.reset( new FakeDriver( m_engine, engine_process_callback, this ) );
        } else {
            ERRORLOG( "Unknown driver " + sDriver );
            audioEngine_raiseError( Engine::UNKNOWN_DRIVER );
        }

        if ( pDriver ) {
            // initialize the audio driver
            int res = pDriver->init( pPref->m_nBufferSize );
            if ( res != 0 ) {
                ERRORLOG( "Error starting audio driver [audioDriver::init()]" );
		pDriver.reset();
            }
        }

        return pDriver;
    }


    /// Start all audio drivers
    void EnginePrivate::audioEngine_startAudioDrivers()
    {
	T<Preferences>::shared_ptr preferencesMng = m_engine->get_preferences();

        m_engine->lock( RIGHT_HERE );
        QMutexLocker mx(&mutex_OutputPointer);

        INFOLOG( "[EnginePrivate::audioEngine_startAudioDrivers]" );

        // check current state
        if ( m_audioEngineState != Engine::StateInitialized ) {
            ERRORLOG( QString( "Error the audio engine is not in INITIALIZED"
                               " state. state=%1" )
                      .arg( m_audioEngineState ) );
            m_engine->unlock();
            return;
        }

        if ( m_pAudioDriver ) { // check if the audio m_pAudioDriver is still alive
            ERRORLOG( "The audio driver is still alive" );
        }
        if ( m_pMidiDriver ) {  // check if midi driver is still alive
            ERRORLOG( "The MIDI driver is still active" );
        }


        QString sAudioDriver = preferencesMng->m_sAudioDriver;
//      sAudioDriver = "Auto";
        if ( sAudioDriver == "Auto" ) {
            if ( ( m_pAudioDriver = createDriver( "Jack" ) ) == NULL ) {
                audioEngine_raiseError( Engine::ERROR_STARTING_DRIVER );
                ERRORLOG( "Error starting audio driver" );
                ERRORLOG( "Using the NULL output audio driver" );

                // use the NULL output driver
                m_pAudioDriver.reset( new NullDriver( m_engine, engine_process_callback, this ) );
                m_pAudioDriver->init( 0 );
            }
        } else {
            m_pAudioDriver = createDriver( sAudioDriver );
            if ( ! m_pAudioDriver ) {
                audioEngine_raiseError( Engine::ERROR_STARTING_DRIVER );
                ERRORLOG( "Error starting audio driver" );
                ERRORLOG( "Using the NULL output audio driver" );

                // use the NULL output driver
                m_pAudioDriver.reset( new NullDriver( m_engine, engine_process_callback, this ) );
                m_pAudioDriver->init( 0 );
            }
        }

        if ( preferencesMng->m_sMidiDriver == "JackMidi" ) {
#ifdef JACK_SUPPORT
            m_jack_client->open();
            m_pMidiDriver.reset( new JackMidiDriver(m_jack_client, m_engine) );
            m_pMidiDriver->open();
            m_pMidiDriver->setActive( true );
#endif
        }

        // change the current audio engine state
        if ( m_pSong == NULL ) {
            m_audioEngineState = Engine::StatePrepared;
        } else {
            m_audioEngineState = Engine::StateReady;
        }


        if ( m_audioEngineState == Engine::StatePrepared ) {
            m_engine->get_event_queue()->push_event( EVENT_STATE, Engine::StatePrepared );
        } else if ( m_audioEngineState == Engine::StateReady ) {
            m_engine->get_event_queue()->push_event( EVENT_STATE, Engine::StateReady );
        }

        // Unlocking earlier might execute the jack process() callback before we
        // are fully initialized.
        mx.unlock();
        m_engine->unlock();

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
                m_pAudioDriver.reset( new NullDriver( m_engine, engine_process_callback, this ) );
                mx.unlock();
                m_pAudioDriver->init( 0 );
                m_pAudioDriver->connect();
            }

#warning "Caching output port buffer pointers is deprecated in "        \
    "JACK.  JACK 2.0 will require that output ports get a new "         \
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
	    m_sampler->set_per_instrument_outs_prefader(
		m_preferences->m_nJackTrackOutputMode == Preferences::PRE_FADER
		);

            audioEngine_setupLadspaFX( m_pAudioDriver->getBufferSize() );
        }


    }



/// Stop all audio drivers
    void EnginePrivate::audioEngine_stopAudioDrivers()
    {
        INFOLOG( "[EnginePrivate::audioEngine_stopAudioDrivers]" );

        m_engine->get_transport()->stop();

        if ( ( m_audioEngineState != Engine::StatePrepared )
             && ( m_audioEngineState != Engine::StateReady ) ) {
            ERRORLOG( QString( "Error: the audio engine is not in PREPARED"
                               " or READY state. state=%1" )
                      .arg( m_audioEngineState ) );
            return;
        }

        // change the current audio engine state
        m_audioEngineState = Engine::StateInitialized;
        m_engine->get_event_queue()->push_event( EVENT_STATE, Engine::StateInitialized );

        m_engine->lock( RIGHT_HERE );

        // delete MIDI driver
        if ( m_pMidiDriver ) {
            m_pMidiDriver->close();
	    m_pMidiDriver.reset();
        }

        // delete audio driver
        if ( m_pAudioDriver ) {
            m_pAudioDriver->disconnect();
            QMutexLocker mx( &mutex_OutputPointer );
	    m_pAudioDriver.reset();
            mx.unlock();
        }

#ifdef JACK_SUPPORT
        m_jack_client->close();
#endif

        m_engine->unlock();
    }



/// Restart all audio and midi drivers
    void EnginePrivate::audioEngine_restartAudioDrivers()
    {
        audioEngine_stopAudioDrivers();
        audioEngine_startAudioDrivers();
    }






//----------------------------------------------------------------------------
//
// Implementation of Engine class
//
//----------------------------------------------------------------------------

    Engine::Engine(T<Preferences>::shared_ptr prefs) :
        d(0)
    {
        assert(prefs);
        d = new EnginePrivate(this, prefs);

        INFOLOG( "[Engine]" );

        d->m_event_queue.reset( new EventQueue );
        d->m_action_manager.reset( new ActionManager(this) );

        d->m_pTransport.reset( new H2Transport(this) );

        d->audioEngine_init();
        d->audioEngine_startAudioDrivers();
    }

    EnginePrivate::~EnginePrivate()
    {
        m_pTransport->stop();
        audioEngine_removeSong();
        audioEngine_stopAudioDrivers();
        audioEngine_destroy();
        __kill_instruments();
    }

    Engine::~Engine()
    {
        INFOLOG( "[~Engine]" );
        d->m_pTransport->stop();
        removeSong();
        delete d;
        d = 0;
    }

    T<Preferences>::shared_ptr Engine::get_preferences()
    {
        return d->m_preferences;
    }

    T<Sampler>::shared_ptr Engine::get_sampler()
    {
        return d->m_sampler;
    }

    T<Mixer>::shared_ptr Engine::get_mixer()
    {
	return boost::dynamic_pointer_cast<Mixer>(d->m_mixer);
    }

    T<Transport>::shared_ptr Engine::get_transport()
    {
        return static_cast<T<Transport>::shared_ptr>(d->m_pTransport);
    }

    T<ActionManager>::shared_ptr Engine::get_action_manager()
    {
        return d->m_action_manager;
    }

    T<EventQueue>::shared_ptr Engine::get_event_queue()
    {
        return d->m_event_queue;
    }

    Playlist& Engine::get_playlist()
    {
        return *d->m_playlist;
    }

#ifdef LADSPA_SUPPORT
    T<Effects>::shared_ptr Engine::get_effects()
    {
        return d->m_effects;
    }
#endif

    void Engine::lock( const char* file, unsigned int line, const char* function )
    {
        d->__engine_mutex.lock();
        d->__locker.file = file;
        d->__locker.line = line;
        d->__locker.function = function;
    }



    bool Engine::try_lock( const char* file, unsigned int line, const char* function )
    {
        bool locked = d->__engine_mutex.tryLock();
        if ( ! locked ) {
            // Lock not obtained
            return false;
        }
        d->__locker.file = file;
        d->__locker.line = line;
        d->__locker.function = function;
        return true;
    }



    void Engine::unlock()
    {
        // Leave "d->__locker" dirty.
        d->__engine_mutex.unlock();
    }

/// Start the internal sequencer
    void Engine::sequencer_play()
    {
        d->m_pTransport->start();
    }

/// Stop the internal sequencer
    void Engine::sequencer_stop()
    {
        d->m_pTransport->stop();
    }



    void Engine::setSong( T<Song>::shared_ptr pSong )
    {
        while( d->m_pSong != 0 ) {
            removeSong();
        }
        d->audioEngine_setSong( pSong );
    }



    void Engine::removeSong()
    {
        d->audioEngine_removeSong();
    }



    T<Song>::shared_ptr Engine::getSong()
    {
        return d->m_pSong;
    }



    void Engine::midi_noteOn( Note *note )
    {
        d->audioEngine_noteOn( note );
    }



    void Engine::midi_noteOff( Note *note )
    {
        d->audioEngine_noteOff( note );
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
        T<Preferences>::shared_ptr pref = get_preferences();
        T<Instrument>::shared_ptr i = d->m_sampler->get_instrument_list()->get(instrument);
        Note note( i,
                   velocity,
                   pan_L,
                   pan_R,
                   -1
            );
        d->m_GuiInput.note_on(&note, pref->getQuantizeEvents());
#warning "JACK MIDI note timing is getting lost here"
    }



    float Engine::getMasterPeak_L()
    {
        return d->m_fMasterPeak_L;
    }



    float Engine::getMasterPeak_R()
    {
        return d->m_fMasterPeak_R;
    }



    unsigned long Engine::getTickPosition()
    {
        TransportPosition pos;
        d->m_pTransport->get_position(&pos);
        return pos.tick + (pos.beat-1) * pos.ticks_per_beat;
    }

    T<PatternList>::shared_ptr Engine::getCurrentPatternList()
    {
        TransportPosition pos;
        d->m_pTransport->get_position(&pos);
        if( pos.bar <= d->m_pSong->get_pattern_group_vector()->size() ) {
            return d->m_pSong->get_pattern_group_vector()->at(pos.bar-1);
        } else {
            return T<PatternList>::shared_ptr();
        }
    }

    T<PatternList>::shared_ptr Engine::getNextPatterns()
    {
        static T<PatternList>::shared_ptr the_nothing(new PatternList);
        TransportPosition pos;
        d->m_pTransport->get_position(&pos);
        size_t p_sz = d->m_pSong->get_pattern_group_vector()->size();
        if( pos.bar < p_sz ) {
            return d->m_pSong->get_pattern_group_vector()->at(pos.bar);
        } else {
            if( d->m_pSong->is_loop_enabled() && p_sz ) {
                return d->m_pSong->get_pattern_group_vector()->at(0);
            } else  {
                return the_nothing;
            }
        }
    }

/// Set the next pattern (Pattern mode only)
    void Engine::sequencer_setNextPattern( int pos, bool /*appendPattern*/, bool /*deletePattern*/ )
    {
        d->m_pSong->set_next_pattern(pos);
    }



    int Engine::getPatternPos()
    {
        TransportPosition pos;
        d->m_pTransport->get_position(&pos);
        return pos.bar-1;
    }



    void Engine::restartDrivers()
    {
        d->audioEngine_restartAudioDrivers();
    }



/// Export a song to a wav file, returns the elapsed time in mSec
    void Engine::startExportSong( const QString& filename )
    {
        d->m_pTransport->stop();
	T<Preferences>::shared_ptr pPref = get_preferences();

        d->m_oldEngineMode = d->m_pSong->get_mode();
        d->m_bOldLoopEnabled = d->m_pSong->is_loop_enabled();

        d->m_pSong->set_mode( Song::SONG_MODE );
        d->m_pSong->set_loop_enabled( false );
        unsigned nSamplerate = d->m_pAudioDriver->getSampleRate();

        // stop all audio drivers
        d->audioEngine_stopAudioDrivers();

        /*
          FIXME: Questo codice fa davvero schifo....
        */


        d->m_pAudioDriver.reset( new DiskWriterDriver( d->m_engine, engine_process_callback, d, nSamplerate, filename ) );

        get_sampler()->stop_playing_notes();

        // reset
        d->m_pTransport->locate( 0 );

        int res = d->m_pAudioDriver->init( pPref->m_nBufferSize );
        if ( res != 0 ) {
            ERRORLOG( "Error starting disk writer driver "
                      "[DiskWriterDriver::init()]" );
        }

        d->m_pMainBuffer_L = d->m_pAudioDriver->getOut_L();
        d->m_pMainBuffer_R = d->m_pAudioDriver->getOut_R();

        d->audioEngine_setupLadspaFX( d->m_pAudioDriver->getBufferSize() );

        d->m_pTransport->locate(0);

        res = d->m_pAudioDriver->connect();
        if ( res != 0 ) {
            ERRORLOG( "Error starting disk writer driver "
                      "[DiskWriterDriver::connect()]" );
        }
    }



    void Engine::stopExportSong()
    {
        if ( ! dynamic_cast<DiskWriterDriver*>(d->m_pAudioDriver.get()) ) {
            return;
        }

//      audioEngine_stopAudioDrivers();
        d->m_pAudioDriver->disconnect();

        d->m_audioEngineState = Engine::StateInitialized;
	d->m_pAudioDriver.reset();

        d->m_pMainBuffer_L = NULL;
        d->m_pMainBuffer_R = NULL;

        d->m_pSong->set_mode( d->m_oldEngineMode );
        d->m_pSong->set_loop_enabled( d->m_bOldLoopEnabled );

        d->audioEngine_startAudioDrivers();

    }



/// Used to display audio driver info
    T<AudioOutput>::shared_ptr Engine::get_audio_output()
    {
        return d->m_pAudioDriver;
    }



/// Used to display midi driver info
    T<MidiInput>::shared_ptr Engine::get_midi_input()
    {
        return d->m_pMidiDriver;
    }



    void Engine::setMasterPeak_L( float value )
    {
        d->m_fMasterPeak_L = value;
    }



    void Engine::setMasterPeak_R( float value )
    {
        d->m_fMasterPeak_R = value;
    }



    Engine::state_t Engine::getState()
    {
        return d->m_audioEngineState;
    }

    float Engine::getProcessTime()
    {
        return d->m_fProcessTime;
    }



    float Engine::getMaxProcessTime()
    {
        return d->m_fMaxProcessTime;
    }



    int Engine::loadDrumkit( T<Drumkit>::shared_ptr drumkitInfo )
    {
        Engine::state_t old_ae_state = d->m_audioEngineState;
        if( d->m_audioEngineState >= Engine::StateReady ) {
            d->m_audioEngineState = Engine::StatePrepared;
        }

        INFOLOG( drumkitInfo->getName() );
        d->m_currentDrumkit = drumkitInfo->getName();
        LocalFileMng fileMng(this);
        QString sDrumkitPath = fileMng.getDrumkitDirectory( drumkitInfo->getName() );


        //current instrument list
        T<InstrumentList>::shared_ptr currInstrList = d->m_sampler->get_instrument_list();

        //new instrument list
        T<InstrumentList>::shared_ptr pDrumkitInstrList = drumkitInfo->getInstrumentList();

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
          while ( pDrumkitInstrList->get_size() < currInstrList->get_size() )
          {
          currInstrList->del(currInstrList->get_size() - 1);
          }
        */

        //needed for the new delete function
        int instrumentDiff =  currInstrList->get_size() - pDrumkitInstrList->get_size();

        for ( unsigned nInstr = 0; nInstr < pDrumkitInstrList->get_size(); ++nInstr ) {
            T<Instrument>::shared_ptr pInstr;
            if ( nInstr < currInstrList->get_size() ) {
                //instrument exists already
                pInstr = currInstrList->get( nInstr );
                assert( pInstr );
            } else {
                pInstr = Instrument::create_empty();
                // The instrument isn't playing yet; no need for locking
                // :-) - Jakob Lund.  lock(
                // "Engine::loadDrumkit" );
                currInstrList->add( pInstr );
                // unlock();
            }

            T<Instrument>::shared_ptr pNewInstr = pDrumkitInstrList->get( nInstr );
            assert( pNewInstr );
            INFOLOG( QString( "Loading instrument (%1 of %2) [%3]" )
                     .arg( nInstr )
                     .arg( pDrumkitInstrList->get_size() )
                     .arg( pNewInstr->get_name() ) );

            // creo i nuovi layer in base al nuovo strumento
            // Moved code from here right into the Instrument class - Jakob Lund.
            pInstr->load_from_placeholder( this, pNewInstr );
        }


//wolke: new delete funktion
        if ( instrumentDiff >=0 ){
            for ( int i = 0; i < instrumentDiff ; i++ ){
                removeInstrument(
                    d->m_sampler->get_instrument_list()->get_size() - 1,
                    true
                    );
            }
        }

#ifdef JACK_SUPPORT
        lock( RIGHT_HERE );
        renameJackPorts();
        unlock();
#endif

        d->m_audioEngineState = old_ae_state;

        return 0;       //ok
    }


//this is also a new function and will used from the new delete function in
//Engine::loadDrumkit to delete the instruments by number
    void Engine::removeInstrument( int instrumentnumber, bool conditional )
    {
        T<Instrument>::shared_ptr pInstr = d->m_sampler->get_instrument_list()->get( instrumentnumber );


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
            getSong()->purge_instrument( pInstr, this );
        }

        T<Song>::shared_ptr pSong = getSong();
        T<InstrumentList>::shared_ptr pList = d->m_sampler->get_instrument_list();
        if(pList->get_size()==1){
            lock( RIGHT_HERE );
            T<Instrument>::shared_ptr zInstr = pList->get( 0 );
            zInstr->set_name( (QString( "Instrument 1" )) );
            // remove all layers
            for ( int nLayer = 0; nLayer < MAX_LAYERS; nLayer++ ) {
                InstrumentLayer* pLayer = zInstr->get_layer( nLayer );
                delete pLayer;
                zInstr->set_layer( NULL, nLayer );
            }
            unlock();
            get_event_queue()->push_event( EVENT_SELECTED_INSTRUMENT_CHANGED, -1 );
            INFOLOG("clear last instrument to empty instrument 1 instead delete the last instrument");
            return;
        }

        // if the instrument was the last on the instruments list, select the
        // next-last
        if ( instrumentnumber >= pList->get_size() - 1 ) {
            setSelectedInstrumentNumber(std::max(0, instrumentnumber - 1));
        }
        // delete the instrument from the instruments list
        lock( RIGHT_HERE );
        pList->del( instrumentnumber );
        getSong()->set_modified(true);
        unlock();

        // At this point the instrument has been removed from both the
        // instrument list and every pattern in the song.  Hence there's no way
        // (NOTE) to play on that instrument, and once all notes have stopped
        // playing it will be save to delete.
        // the ugly name is just for debugging...
        QString xxx_name = QString( "XXX_%1" ) . arg( pInstr->get_name() );
        pInstr->set_name( xxx_name );
        d->__instrument_death_row.push_back( pInstr );
        d->__kill_instruments(); // checks if there are still notes.

        // this will force a GUI update.
        get_event_queue()->push_event( EVENT_SELECTED_INSTRUMENT_CHANGED, -1 );
    }

    const QString& Engine::getCurrentDrumkitname()
    {
        return d->m_currentDrumkit;
    }

    void Engine::setCurrentDrumkitname(const QString& currentdrumkitname )
    {
        d->m_currentDrumkit = currentdrumkitname;
    }

    void Engine::raiseError( Engine::error_t nErrorCode )
    {
        d->audioEngine_raiseError( nErrorCode );
    }


    unsigned long Engine::getRealtimeFrames()
    {
        return d->m_nFreeRollingFrameCounter;
    }

/**
 * Get the ticks for pattern at pattern pos
 * @a int pos -- position in song
 * @return -1 if pos > number of patterns in the song, tick no. > 0 otherwise
 * The driver should be LOCKED when calling this!!
 */
    long Engine::getTickForPosition( int pos )
    {
        int nPatternGroups = d->m_pSong->get_pattern_group_vector()->size();
        if( nPatternGroups == 0 ) return -1;

        if ( pos >= nPatternGroups ) {
            if ( d->m_pSong->is_loop_enabled() ) {
                pos = pos % nPatternGroups;
            } else {
                WARNINGLOG( QString( "patternPos > nPatternGroups. pos:"
                                     " %1, nPatternGroups: %2")
                            .arg( pos )
                            .arg(  nPatternGroups ) );
                return -1;
            }
        }

        T<Song::pattern_group_t>::shared_ptr pColumns = d->m_pSong->get_pattern_group_vector();
        long totalTick = 0;
        int nPatternSize;
        T<Pattern>::shared_ptr pPattern;
        for ( int i = 0; i < pos; ++i ) {
            T<PatternList>::shared_ptr pColumn = ( *pColumns )[ i ];
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
        d->m_pTransport->locate(pos+1, 1, 0);
    }

    void Engine::getLadspaFXPeak( int nFX, float *fL, float *fR )
    {
#ifdef LADSPA_SUPPORT
        ( *fL ) = d->m_fFXPeak_L[nFX];
        ( *fR ) = d->m_fFXPeak_R[nFX];
#else
        ( *fL ) = 0;
        ( *fR ) = 0;
#endif
    }



    void Engine::setLadspaFXPeak( int nFX, float fL, float fR )
    {
#ifdef LADSPA_SUPPORT
        d->m_fFXPeak_L[nFX] = fL;
        d->m_fFXPeak_R[nFX] = fR;
#endif
    }


    void Engine::onTapTempoAccelEvent()
    {
        d->m_BeatCounter.onTapTempoAccelEvent();
    }

    void Engine::setTapTempo( float fInterval )
    {
        d->m_BeatCounter.setTapTempo(fInterval);
    }


    void Engine::setBPM( float fBPM )
    {
        if( (fBPM < 500.0) && (fBPM > 20.0) ) {
            d->m_pSong->set_bpm(fBPM);
        }
    }



    void Engine::restartLadspaFX()
    {
        if ( d->m_pAudioDriver ) {
            lock( RIGHT_HERE );
            d->audioEngine_setupLadspaFX( d->m_pAudioDriver->getBufferSize() );
            unlock();
        } else {
            ERRORLOG( "m_pAudioDriver = NULL" );
        }
    }



    int Engine::getSelectedPatternNumber()
    {
        return d->m_nSelectedPatternNumber;
    }



    void Engine::setSelectedPatternNumber( int nPat )
    {
        // FIXME: controllare se e' valido..
        if ( nPat == d->m_nSelectedPatternNumber )      return;


        if ( get_preferences()->patternModePlaysSelected() ) {
            lock( RIGHT_HERE );

            d->m_nSelectedPatternNumber = nPat;
            unlock();
        } else {
            d->m_nSelectedPatternNumber = nPat;
        }

        get_event_queue()->push_event( EVENT_SELECTED_PATTERN_CHANGED, -1 );
    }



    int Engine::getSelectedInstrumentNumber()
    {
        return d->m_nSelectedInstrumentNumber;
    }



    void Engine::setSelectedInstrumentNumber( int nInstrument )
    {
        if ( d->m_nSelectedInstrumentNumber == nInstrument )    return;

        d->m_nSelectedInstrumentNumber = nInstrument;
        get_event_queue()->push_event( EVENT_SELECTED_INSTRUMENT_CHANGED, -1 );
    }


#ifdef JACK_SUPPORT
    void Engine::renameJackPorts()
    {
        if( get_preferences()->m_bJackTrackOuts == true ){
            d->audioEngine_renameJackPorts();
	    d->m_sampler->set_per_instrument_outs(true);
	    d->m_sampler->set_per_instrument_outs_prefader(
		get_preferences()->m_nJackTrackOutputMode == Preferences::PRE_FADER
		);
        } else {
	    d->m_sampler->set_per_instrument_outs(false);
	}
    }
#endif


///BeatCounter

    void Engine::setbeatsToCount( int beatstocount)
    {
        d->m_BeatCounter.setBeatsToCount(beatstocount);
    }


    int Engine::getbeatsToCount()
    {
        return d->m_BeatCounter.getBeatsToCount();
    }


    void Engine::setNoteLength( float notelength)
    {
        d->m_BeatCounter.setNoteLength(notelength);
    }



    float Engine::getNoteLength()
    {
        return d->m_BeatCounter.getNoteLength();
    }



    int Engine::getBcStatus()
    {
        return d->m_BeatCounter.status();
    }


    void Engine::setBcOffsetAdjust()
    {
        d->m_BeatCounter.setOffsetAdjust();
    }


    void Engine::handleBeatCounter()
    {
        d->m_BeatCounter.trigger();
    }
//~ beatcounter

// jack transport master

    bool Engine::setJackTimeMaster(bool if_none_already)
    {
        return d->m_pTransport->setJackTimeMaster(d->m_jack_client, if_none_already);
    }

    void Engine::clearJackTimeMaster()
    {
        d->m_pTransport->clearJackTimeMaster();
    }

    bool Engine::getJackTimeMaster()
    {
        return d->m_pTransport->getJackTimeMaster();
    }

//~ jack transport master

/**
 * Toggles between SINGLE-PATTERN pattern mode, and STACKED pattern
 * mode.  In stacked pattern mode, more than one pattern may be
 * playing at once.  Also called "Live" mode.
 */
    void Engine::togglePlaysSelected()
    {
        T<Preferences>::shared_ptr  P = get_preferences();
        bool isPlaysSelected = P->patternModePlaysSelected();

        // NEED TO IMPLEMENT!!
        assert(false);

        P->setPatternModePlaysSelected( !isPlaysSelected );

    }

    Engine::playlist_t& Engine::get_internal_playlist()
    {
        return d->m_Playlist;
    }

#ifdef JACK_SUPPORT
    int jackMidiFallbackProcess(jack_nframes_t nframes, void* arg)
    {
        EnginePrivate *d = static_cast<EnginePrivate*>(arg);
        JackMidiDriver* instance =
            dynamic_cast<JackMidiDriver*>(d->m_pMidiDriver.get());
        return instance->processNonAudio(nframes);
    }
#endif

    void EnginePrivate::__kill_instruments()
    {
	// TODO: Deleting all instruments can be done with
	// simply __instrument_death_row.clear().  But,
	// apparently notes get added to this list before
	// they should /actually/ be deleted.... thus the
	// check for pInstr->is_queued().
        int c = 0;
        T<Instrument>::shared_ptr pInstr;
        while ( __instrument_death_row.size()
                && __instrument_death_row.front()->is_queued() == 0 ) {
            pInstr = __instrument_death_row.front();
            __instrument_death_row.pop_front();
            INFOLOG( QString( "Deleting unused instrument (%1). "
                              "%2 unused remain." )
                     . arg( pInstr->get_name() )
                     . arg( __instrument_death_row.size() ) );
            pInstr.reset();
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
        get_sampler()->stop_playing_notes();
    }

    void Engine::set_last_midi_event(QString const& event, int data)
    {
        d->lastMidiEvent = event;
        d->lastMidiEventParameter = data;
    }

    bool Engine::have_last_midi_event()
    {
        return !d->lastMidiEvent.isEmpty();
    }

    void Engine::get_last_midi_event(QString* event, int* param)
    {
        *event = d->lastMidiEvent;
        *param = d->lastMidiEventParameter;
    }

} // namespace Tritium
