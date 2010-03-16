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
#ifndef TRITIUM_ENGINE_HPP
#define TRITIUM_ENGINE_HPP

#include <stdint.h> // for uint32_t et al
#include <Tritium/EngineInterface.hpp>
#include <Tritium/Song.hpp>
#include <Tritium/memory.hpp>
#include <QMutex>
#include <vector>
#include <list>
#include <cassert>


/**
 * Convenience macro for locking the Engine.
 */
#ifndef RIGHT_HERE
#define RIGHT_HERE __FILE__, __LINE__, __PRETTY_FUNCTION__
#endif

namespace Tritium
{

    class ActionManager;
    class AudioEngine;
    class AudioOutput;
    class Drumkit;
    class Effects;
    class EventQueue;
    class MidiInput;
    class MidiMap;
    class Playlist;
    class Preferences;
    class Sampler;
    class Mixer;
    class Transport;

    class EnginePrivate;

    /**
     * \brief This is the main Tritium Engine.
     */
    class Engine : public EngineInterface
    {
    public:
        // The preferences object must be created and
        // initialized before Engine is created
        // (for now).  Engine takes ownership and
        // will delete it.
        Engine(T<Preferences>::shared_ptr prefs);
        ~Engine();

	///////////////////////////////////////
	// ACCESS TO MAJOR COMPONENTS
	///////////////////////////////////////

	T<Transport>::shared_ptr get_transport(); // Never returns null
	T<Preferences>::shared_ptr get_preferences();
	T<AudioOutput>::shared_ptr get_audio_output();
	T<MidiInput>::shared_ptr get_midi_input();
	T<ActionManager>::shared_ptr get_action_manager();
	T<Sampler>::shared_ptr get_sampler();
	T<Mixer>::shared_ptr get_mixer();
	T<EventQueue>::shared_ptr get_event_queue();
        Playlist& get_playlist();
#ifdef LADSPA_SUPPORT
	T<Effects>::shared_ptr get_effects();
#endif

	///////////////////////////////////////
	// ENGINE STATE AND ERRORS
	///////////////////////////////////////

	/**
	 * \brief The status of the Engine.
	 *
	 * It's OK to use ==, <, and > when testing.
	 */
	typedef enum {
	    StateUninitialized = 1, ///< Not even contructors have been called
	    StateInitialized = 2,   ///< Not ready, but most pointers are valid
	    StatePrepared = 3,      ///< Drivers set up, but not ready for audio
	    StateReady = 4,         ///< Ready to process audio
	} state_t;

        state_t getState();

        typedef enum {
            UNKNOWN_DRIVER,
            ERROR_STARTING_DRIVER,
            JACK_SERVER_SHUTDOWN,
            JACK_CANNOT_ACTIVATE_CLIENT,
            JACK_CANNOT_CONNECT_OUTPUT_PORT,
            JACK_ERROR_IN_PORT_REGISTER,
        } error_t;

        void raiseError( error_t nErrorCode );


	///////////////////////////////////////
	// THE BIG LOCK
	///////////////////////////////////////
        /* Mutex locking and unlocking
         *
         * Easy usage:  Use the RIGHT_HERE macro like this...
         *     engine->lock( RIGHT_HERE );
         *
         * More complex usage:  The parameters file and function
         * need to be pointers to null-terminated strings that are
         * persistent for the entire session.  This does *not*
         * include the return value of std::string::c_str(), or
         * QString::toLocal8Bit().data().
         *
         * Notes: The order of the parameters match GCC's
         * implementation of the assert() macros.
         */
        void lock( const char* file, unsigned int line, const char* function );
        bool try_lock( const char* file, unsigned int line, const char* function );
        void unlock();

	///////////////////////////////////////
	// SEQUENCER CONTROLS
	///////////////////////////////////////

        /// Start the internal sequencer
        void sequencer_play();

        /// Stop the internal sequencer
        void sequencer_stop();

        void __panic();

        void midi_noteOn( Note *note );
        void midi_noteOff( Note *note );

        void sequencer_setNextPattern(
	    int pos,
	    bool appendPattern,
	    bool deletePattern );
        void togglePlaysSelected();

        void addRealtimeNote (
	    int instrument,
	    float velocity,
	    float pan_L=1.0,
	    float pan_R=1.0,
	    float pitch=0.0,
	    bool forcePlay=false,
	    bool use_frame = false,
	    uint32_t frame = 0 );

        unsigned long getTickPosition();
        unsigned long getRealtimeFrames();

        T<PatternList>::shared_ptr getCurrentPatternList();
        T<PatternList>::shared_ptr getNextPatterns();

        int getPatternPos();
        void setPatternPos( int pos );

        long getTickForPosition( int );

        int getSelectedPatternNumber();
        void setSelectedPatternNumber( int nPat );

        int getSelectedInstrumentNumber();
        void setSelectedInstrumentNumber( int nInstrument );

        /// jack time master
	// Returns true if we have become the master
        bool setJackTimeMaster(bool if_none_already = false);
        void clearJackTimeMaster();
        /* Note:  There's no way to know for sure
	   if we are _actually_ the JACK time master. */
        bool getJackTimeMaster();
        ///~jack time master

	// TODO: Remove this function.  This is a workaround
	// for the old code which manipulated the internal
	// class members.  This should be done somewhere
	// else than in Tritium::Engine.
	void set_last_midi_event(const QString& ev, int param = -1);
	bool have_last_midi_event(); // i.e. event is non-empty
	void get_last_midi_event(QString* event, int* param);

	///////////////////////////////////////
	// SONG CONTROLS
	///////////////////////////////////////

        /// Set current song
        void setSong( T<Song>::shared_ptr newSong );

        /// Return the current song
	T<Song>::shared_ptr getSong();
        void removeSong();

	///////////////////////////////////////
	// MIXER CONTROLS
	///////////////////////////////////////

        float getMasterPeak_L();
        void setMasterPeak_L( float value );

        float getMasterPeak_R();
        void setMasterPeak_R( float value );

        void getLadspaFXPeak( int nFX, float *fL, float *fR );
        void setLadspaFXPeak( int nFX, float fL, float fR );

	///////////////////////////////////////
	// AUDIO DRIVER CONTROLS
	///////////////////////////////////////

        void restartDrivers();

        void startExportSong( const QString& filename );
        void stopExportSong();

        float getProcessTime();
        float getMaxProcessTime();

#ifdef JACK_SUPPORT
        void renameJackPorts();
#endif

	///////////////////////////////////////
	// SAMPLER CONTROL
	///////////////////////////////////////

        int loadDrumkit( T<Drumkit>::shared_ptr drumkitInfo );

        /// delete an instrument. If `conditional` is true, and there
        /// are patterns that use this instrument, it's not deleted
        /// anyway
        void removeInstrument( int instrumentnumber, bool conditional );

        const QString& getCurrentDrumkitname();
        void setCurrentDrumkitname( const QString& currentdrumkitname );

        void previewSample( T<Sample>::shared_ptr pSample );
        void previewInstrument( T<Instrument>::shared_ptr pInstr );

	///////////////////////////////////////
	// TEMPO CONTROLS
	///////////////////////////////////////

        void onTapTempoAccelEvent();
        void setTapTempo( float fInterval );
        void setBPM( float fBPM );

        ///beatconter
        void setbeatsToCount( int beatstocount);
        int getbeatsToCount();
        void setNoteLength( float notelength);
        float getNoteLength();
        int getBcStatus();
        void handleBeatCounter();
        void setBcOffsetAdjust();

	///////////////////////////////////////
	// EFFECTS
	///////////////////////////////////////

        void restartLadspaFX();

	///////////////////////////////////////
	// PLAYLIST
	///////////////////////////////////////

        ///playlist vector
        struct HPlayListNode
        {
            QString m_hFile;
            QString m_hScript;
            QString m_hScriptEnabled;
        };

	typedef std::vector<HPlayListNode> playlist_t;

	// TODO: This is a workaround function.  It needs to be
	// removed and replaced by something that is a little
	// better designed.  Why have Tritium::Playlist if
	// everyone is manipulating the internale playlist_t vector?
	playlist_t& get_internal_playlist();

    private:
        EnginePrivate* d;
    };

    /**
     * \brief Global pointer to engine (convenience)
     *
     * This pointer is provided for the convenience of client applications
     * that wish to have a single, global instance of the Tritium engine.
     * This is just a declaration.  You must create the actual variable
     * and instance somewhere else.
     *
     * This global variable IS NOT used internally by Tritium at all.
     */
    extern Engine* g_engine;

} // namespace Tritium

#endif  // TRITIUM_ENGINE_HPP
