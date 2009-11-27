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

#ifndef TRITIUM_SONG_HPP
#define TRITIUM_SONG_HPP


#include <QString>
#include <QDomNode>
#include <vector>
#include <map>

#include <Tritium/Logger.hpp>

class TiXmlNode;

namespace Tritium
{

class ADSR;
class Sample;
class Note;
class Instrument;
class InstrumentList;
class Pattern;
class Song;
class PatternList;
class PatternModeManager;  // Private to Song.  See song.cpp

/**
\ingroup H2CORE
\brief	Song class
*/
class Song : public Object
{
public:
	enum SongMode {
		PATTERN_MODE,
		SONG_MODE
	};

	bool __is_muted;
	unsigned __resolution;		///< Resolution of the song (number of ticks per quarter)
	float __bpm;			///< Beats per minute
	bool __is_modified;
	QString __name;		///< song name
	QString __author;	///< author of the song
	QString __license;	///< license of the song

	/*
	// internal delay FX
	bool m_bDelayFXEnabled;
	float m_fDelayFXWetLevel;
	float m_fDelayFXFeedback;
	unsigned m_nDelayFXTime;
	//~ internal delay fx
	*/

	static Song* get_empty_song();
	static Song* get_default_song();

	Song( const QString& name, const QString& author, float bpm, float volume );
	~Song();
	
	/**
	  Remove all the notes in the song that play on instrument I.
	  The function is real-time safe (it locks the audio data while deleting notes)
	*/
	void purge_instrument( Instrument* I );

	void set_volume( float volume ) {
		__volume = volume;
	}
	float get_volume() {
		return __volume;
	}

	void set_metronome_volume( float volume ) {
		__metronome_volume = volume;
	}
	float get_metronome_volume() {
		return __metronome_volume;
	}

	PatternList* get_pattern_list() {
		return __pattern_list;
	}
	void set_pattern_list( PatternList *pattern_list ) {
		__pattern_list = pattern_list;
	}

	std::vector<PatternList*>* get_pattern_group_vector() {
		return __pattern_group_sequence;
	}
	void set_pattern_group_vector( std::vector<PatternList*>* vect ) {
		__pattern_group_sequence = vect;
	}

	static Song* load( const QString& sFilename );
	bool save( const QString& sFilename );

	InstrumentList* get_instrument_list() {
		return __instrument_list;
	}
	void set_instrument_list( InstrumentList *list ) {
		__instrument_list = list;
	}


	void set_notes( const QString& notes ) {
		__notes = notes;
	}
	const QString& get_notes() {
		return __notes;
	}

	void set_license( const QString& license ) {
		__license = license;
	}
	const QString& get_license() {
		return __license;
	}

	const QString& get_filename() {
		return __filename;
	}
	void set_filename( const QString& filename ) {
		__filename = filename;
	}

	bool is_loop_enabled() {
		return __is_loop_enabled;
	}
	void set_loop_enabled( bool enabled ) {
		__is_loop_enabled = enabled;
	}

	float get_humanize_time_value() {
		return __humanize_time_value;
	}
	void set_humanize_time_value( float value ) {
		__humanize_time_value = value;
	}

	float get_humanize_velocity_value() {
		return __humanize_velocity_value;
	}
	void set_humanize_velocity_value( float value ) {
		__humanize_velocity_value = value;
	}

	float get_swing_factor() {
		return __swing_factor;
	}
	void set_swing_factor( float factor );

	SongMode get_mode() {
		return __song_mode;
	}
	void set_mode( SongMode mode ) {
		__song_mode = mode;
	}

	// PATTERN MODE METHODS
	// ====================

	// NOTE:  Pattern Mode options (lists of patterns, pattern mode
	// type) does not persist with the Song file.  These are set
	// and manipulated by the session.

	enum PatternModeType { SINGLE, STACKED };

	PatternModeType get_pattern_mode_type();
	void set_pattern_mode_type(PatternModeType t);
	void toggle_pattern_mode_type();

	// Manipulate the pattern lists and queues.
	// Patterns may only be added/removed once, so subsequent add/remove
	// operations will have no affect.
	// If 'pos' is not in the range 0 <= pos <= __pattern_list->get_size(),
	// these will silently ignore the request.
	void append_pattern(int pos);      // Appends pattern to the current group on next cycle.
	void remove_pattern(int pos);      // Remove the pattern from the current group on next cycle.
	void reset_patterns();             // Clears out the current and "next" queues.
	void set_next_pattern(int pos);    // Sched. a pattern to replace the current group.
	                                   // ...clears out any that are currently queued.
	void append_next_pattern(int pos); // Adds pattern to the "next" queued patterns.
	void remove_next_pattern(int pos); // Removes pattern from the "next" queue
	void clear_queued_patterns();      // Clears out the "next" queued patterns.

	// Copies the currently playing patterns into rv.
	// This only makes sense in pattern mode.  Otherwise,
	// this function returns nonsense.
	void get_playing_patterns(PatternList& rv);

	// This method should *ONLY* be used by the sequencer.
	// This signals to the Song class that the current pattern
	// is done playing, and to switch to the next pattern if
	// there are any queued.
	void go_to_next_patterns();

	//~PATTERN MODE METHODS

private:
	float __volume;						///< volume of the song (0.0..1.0)
	float __metronome_volume;				///< Metronome volume
	QString __notes;
	PatternList *__pattern_list;				///< Pattern list
	std::vector<PatternList*>* __pattern_group_sequence;	///< Sequence of pattern groups
	InstrumentList *__instrument_list;			///< Instrument list
	QString __filename;
	bool __is_loop_enabled;
	float __humanize_time_value;
	float __humanize_velocity_value;
	float __swing_factor;

	SongMode __song_mode;

	PatternModeManager* __pat_mode;
};



/**
\ingroup H2CORE
\brief	Read XML file of a song
*/
class SongReader : public Object
{
public:
	SongReader();
	~SongReader();
	Song* readSong( const QString& filename );

private:
	QString m_sSongVersion;

	/// Dato un XmlNode restituisce un oggetto Pattern
	Pattern* getPattern( QDomNode pattern, InstrumentList* instrList );
};


} // namespace Tritium

#endif // TRITIUM_SONG_HPP
