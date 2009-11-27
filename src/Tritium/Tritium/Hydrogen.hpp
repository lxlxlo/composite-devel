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
#ifndef TRITIUM_HYDROGEN_HPP
#define TRITIUM_HYDROGEN_HPP

#include <stdint.h> // for uint32_t et al
#include <Tritium/Action.hpp>
#include <Tritium/Song.hpp>
#include <Tritium/Logger.hpp>
#include <Tritium/IO/AudioOutput.hpp>
#include <Tritium/IO/MidiInput.hpp>
#include <Tritium/SoundLibrary.hpp>
#include <cassert>

// Engine states  (It's ok to use ==, <, and > when testing)
#define STATE_UNINITIALIZED	1     // Not even the constructors have been called.
#define STATE_INITIALIZED	2     // Not ready, but most pointers are now valid or NULL
#define STATE_PREPARED		3     // Drivers are set up, but not ready to process audio.
#define STATE_READY		4     // Ready to process audio
/*
#define STATE_PLAYING		5     // Currently playing a sequence.
*/

inline int randomValue( int max );

namespace Tritium
{

class Transport;

///
/// Hydrogen Audio Engine.
///
class Hydrogen : public Object
{
public:
	/// Return the Hydrogen instance
	static void create_instance();  // Also creates other instances, like AudioEngine
	static Hydrogen* get_instance() { assert(__instance); return __instance; };

	~Hydrogen();

// ***** ACCESS TO TRANSPORT ******
	Transport* get_transport(); // Never returns null

// ***** SEQUENCER ********
	/// Start the internal sequencer
	void sequencer_play();

	/// Stop the internal sequencer
	void sequencer_stop();

	void midi_noteOn( Note *note );
	void midi_noteOff( Note *note );

	///Last received midi message
	QString lastMidiEvent;
	int lastMidiEventParameter;


	void sequencer_setNextPattern( int pos, bool appendPattern, bool deletePattern );
	void togglePlaysSelected( void );
// ***** ~SEQUENCER ********

	/// Set current song
	void setSong( Song *newSong );

	/// Return the current song
	Song* getSong();
	void removeSong();

	void addRealtimeNote ( int instrument, float velocity, float pan_L=1.0, float pan_R=1.0, float pitch=0.0, bool forcePlay=false, bool use_frame = false, uint32_t frame = 0 );


	float getMasterPeak_L();
	void setMasterPeak_L( float value );

	float getMasterPeak_R();
	void setMasterPeak_R( float value );

	void getLadspaFXPeak( int nFX, float *fL, float *fR );
	void setLadspaFXPeak( int nFX, float fL, float fR );


	unsigned long getTickPosition();
	unsigned long getRealtimeFrames();


	PatternList * getCurrentPatternList();

	PatternList * getNextPatterns();

	int getPatternPos();
	void setPatternPos( int pos );
	
	long getTickForPosition( int );

	void restartDrivers();

	void startExportSong( const QString& filename );
	void stopExportSong();

	AudioOutput* getAudioOutput();
	MidiInput* getMidiInput();

	int getState();

	float getProcessTime();
	float getMaxProcessTime();

	int loadDrumkit( Drumkit *drumkitInfo );
	
	/// delete an instrument. If `conditional` is true, and there are patterns that
	/// use this instrument, it's not deleted anyway
	void removeInstrument( int instrumentnumber, bool conditional );

	//return the name of the current drumkit
	QString m_currentDrumkit;

	const QString& getCurrentDrumkitname() {
		return m_currentDrumkit;
	}

	void setCurrentDrumkitname( const QString& currentdrumkitname ) {
		this->m_currentDrumkit = currentdrumkitname;
	}

	void raiseError( unsigned nErrorCode );


	void previewSample( Sample *pSample );
	void previewInstrument( Instrument *pInstr );

	enum ErrorMessages {
		UNKNOWN_DRIVER,
		ERROR_STARTING_DRIVER,
		JACK_SERVER_SHUTDOWN,
		JACK_CANNOT_ACTIVATE_CLIENT,
		JACK_CANNOT_CONNECT_OUTPUT_PORT,
		JACK_ERROR_IN_PORT_REGISTER
	};

	void onTapTempoAccelEvent();
	void setTapTempo( float fInterval );
	void setBPM( float fBPM );

	void restartLadspaFX();

	int getSelectedPatternNumber();
	void setSelectedPatternNumber( int nPat );

	int getSelectedInstrumentNumber();
	void setSelectedInstrumentNumber( int nInstrument );
#ifdef JACK_SUPPORT
	void renameJackPorts();
#endif

	///playlist vector
	struct HPlayListNode
	{
		QString m_hFile;
		QString m_hScript;
		QString m_hScriptEnabled;
	};

	std::vector<HPlayListNode> m_PlayList;
	
	///beatconter
	void setbeatsToCount( int beatstocount);
	int getbeatsToCount();
	void setNoteLength( float notelength);
	float getNoteLength();
	int getBcStatus();
	void handleBeatCounter();
	void setBcOffsetAdjust();

	/// jack time master
	bool setJackTimeMaster(bool if_none_already = false);  // Returns true if we became master
	void clearJackTimeMaster();
	bool getJackTimeMaster();    /* Note:  There's no way to know for sure
	                                if we are _actually_ the JACK time master. */
	///~jack time master

	void __panic();
	

private:
	static Hydrogen* __instance;

	// used for song export
	Song::SongMode m_oldEngineMode;
	bool m_bOldLoopEnabled;

	std::list<Instrument*> __instrument_death_row; /// Deleting instruments too soon leads to potential crashes.


	/// Private constructor
	Hydrogen();

	void __kill_instruments();

};

} // namespace Tritium

#endif  // TRITIUM_HYDROGEN_HPP

