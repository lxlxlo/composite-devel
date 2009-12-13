/*
 * Copyright (c) 2009 by Gabriel M. Beddingfield <gabriel@teuton.org>
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
#ifndef TRITIUM_ENGINEPRIVATE_HPP
#define TRITIUM_ENGINEPRIVATE_HPP

#include <Tritium/globals.hpp>
#include "transport/H2Transport.hpp"
#include "BeatCounter.hpp"
#include "SongSequencer.hpp"

#include <Tritium/Transport.hpp>
#include <Tritium/SeqEvent.hpp>
#include <Tritium/SeqScript.hpp>
#include <Tritium/SeqScriptIterator.hpp>

#include <QMutex>

namespace Tritium
{

    class Engine;

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
	Engine *m_engine;
        EvList __events;
        QMutex __mutex;

    public:
	GuiInputQueue(Engine* parent) : m_engine(parent) { assert(parent); }

        int process( SeqScript& seq, const TransportPosition& pos, uint32_t nframes ) {
            // Set up quantization.
            uint32_t quant_frame;

            {
                // TODO:  This seems too complicated for what we're doing...
                Preferences *pref = m_engine->get_preferences();
                TransportPosition quant(pos);
                quant.ceil(TransportPosition::TICK);

                double res = (double)pref->getPatternEditorGridResolution();
                double trip_f = pref->isPatternEditorUsingTriplets() ? (2.0/3.0) : 1.0;
                // Round to scalar * beat resolution
                double fquant_ticks = quant.ticks_per_beat * (4.0 / res) * trip_f;
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
                m_engine->getSong()
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
                m_engine->getSong()
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

    class EnginePrivate
    {
    public:

        void audioEngine_init();
        void audioEngine_destroy();
	void audioEngine_raiseError(unsigned nErrorCode);
        int audioEngine_start(
            bool bLockEngine = false,
            unsigned nTotalFrames = 0 );
        void audioEngine_stop( bool bLockEngine = false );
	AudioOutput* createDriver(const QString& sDriver);
        void audioEngine_setSong( Song *newSong );
	void audioEngine_setupLadspaFX(unsigned nBufferSize );
	void audioEngine_renameJackPorts();
        void audioEngine_removeSong();
        void audioEngine_noteOn( Note *note );
        void audioEngine_noteOff( Note *note );
        int     audioEngine_process( uint32_t nframes );
	inline void audioEngine_process_clearAudioBuffers(uint32_t nFrames);
        inline void audioEngine_clearNoteQueue();
        inline void audioEngine_process_playNotes( unsigned long nframes );
        inline unsigned audioEngine_renderNote(
            Note* pNote,
            const unsigned& nBufferSize );
        inline void audioEngine_updateNoteQueue(
            unsigned nFrames,
            const TransportPosition& pos );
        inline void audioEngine_prepNoteQueue();

        inline int findPatternInTick(
            int tick,
            bool loopMode,
            int *patternStartTick );

        void audioEngine_restartAudioDrivers();
        void audioEngine_startAudioDrivers();
        void audioEngine_stopAudioDrivers();

        void __kill_instruments();

        /////////////////////////////////////////
        // Stuff from the old Tritium::Engine
        /////////////////////////////////////////

        Engine* m_engine;

        ///Last received midi message
        QString lastMidiEvent;
        int lastMidiEventParameter;

        QString m_currentDrumkit;

	////////////////////////////////////////////////////////////
	////////// M U S T        F I X ////////////////////////////
	////////////////////////////////////////////////////////////
	/// M  U  S  T           F   I   X  !  !  //////////////////
	////////////////////////////////////////////////////////////
	// m_Playlist and m_playlist have names that are too      //
	// similar.  They need to be more different.              //
	////////////////////////////////////////////////////////////
        Engine::playlist_t m_Playlist;

        Sampler* __sampler;

        /// Mutex for syncronized access to the Song object and the
        /// AudioEngine.
        QMutex __engine_mutex;

        struct _locker_struct {
            const char* file;
            unsigned int line;
            const char* function;
        } __locker;

        // used for song export
        Song::SongMode m_oldEngineMode;
        bool m_bOldLoopEnabled;

        /**
         * In some cases, deleting a large list of instruments is not
         * realtime safe.
         */
        std::list<Instrument*> __instrument_death_row;

        /////////////////////////////////////////
        // Old Global Varibles from Engine.cpp
        /////////////////////////////////////////

        float m_fMasterPeak_L;           ///< Master peak (left channel)
        float m_fMasterPeak_R;           ///< Master peak (right channel)
        float m_fProcessTime;            ///< time used in process function
        float m_fMaxProcessTime;         ///< max ms usable in process with no xrun

        Preferences* m_preferences;
        ActionManager* m_action_manager;
        Sampler* m_sampler;
        EventQueue* m_event_queue;
        H2Transport* m_pTransport;
        Playlist* m_playlist;
#ifdef JACK_SUPPORT
        JackClient* m_jack_client;
#endif
#ifdef LADSPA_SUPPORT
        Effects* m_effects;
#endif

        // This is *the* priority queue for scheduling notes/events to be
        // sent to the Sampler.
        SeqScript m_queue;
        GuiInputQueue m_GuiInput;
        SongSequencer m_SongSequencer;

        BeatCounter m_BeatCounter;

        AudioOutput *m_pAudioDriver;     ///< Audio output
        MidiInput *m_pMidiDriver;        ///< MIDI input
        QMutex mutex_OutputPointer;             ///< Mutex for audio output pointer, allows multiple readers
        ///< When locking this AND AudioEngine, always lock AudioEngine first.


        Song *m_pSong;                          ///< Current song
        Instrument *m_pMetronomeInstrument;      ///< Metronome instrument
        unsigned long m_nFreeRollingFrameCounter;

// Buffers used in the process function
        float *m_pMainBuffer_L;
        float *m_pMainBuffer_R;


	Engine::state_t  m_audioEngineState;   ///< Audio engine state

        int m_nSelectedPatternNumber;
        int m_nSelectedInstrumentNumber;
        bool m_sendPatternChange;

#ifdef LADSPA_SUPPORT
        float m_fFXPeak_L[MAX_FX];
        float m_fFXPeak_R[MAX_FX];
#endif

	/////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////

	EnginePrivate(Engine* parent, Preferences* prefs) :
	    m_engine(parent),
	    lastMidiEvent(),
	    lastMidiEventParameter(-1),
	    m_currentDrumkit(),
	    m_Playlist(),
	    __sampler(0),
	    __engine_mutex(),
	    m_oldEngineMode(Song::SONG_MODE),
	    m_bOldLoopEnabled(false),
	    __instrument_death_row(),
	    m_fMasterPeak_L(0.0),
	    m_fMasterPeak_R(0.0),
	    m_fProcessTime(0.0),
	    m_fMaxProcessTime(0.0),
	    m_preferences(prefs),
	    m_action_manager(0),
	    m_sampler(0),
	    m_event_queue(0),
	    m_pTransport(0),
	    m_playlist(0),
#ifdef JACK_SUPPORT
	    m_jack_client(0),
#endif
#ifdef LADSPA_SUPPORT
	    m_effects(0),
#endif
	    m_queue(),
	    m_GuiInput(parent),
	    m_SongSequencer(),
	    m_BeatCounter(parent),
	    m_pAudioDriver(0),
	    m_pMidiDriver(0),
	    mutex_OutputPointer(),
	    m_pSong(0),
	    m_pMetronomeInstrument(0),
	    m_nFreeRollingFrameCounter(0),
	    m_pMainBuffer_L(0),
	    m_pMainBuffer_R(0),
	    m_audioEngineState(Engine::StateUninitialized),
	    m_nSelectedPatternNumber(-1),
	    m_nSelectedInstrumentNumber(-1),
	    m_sendPatternChange(false)
	    {
		assert(parent);
		__locker.file = 0;
		__locker.line = 0;
		__locker.function = 0;
	    }

	~EnginePrivate();

    }; // class EnginePrivate

} // namespace Tritium

#endif  // TRITIUM_ENGINEPRIVATE_HPP
