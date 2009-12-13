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
#include <deque>
#include <map>
#include <stdint.h>

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
class Engine;

/**
 *\brief Song (sequence) class.
 */
class Song
{
public:
    class SongPrivate;

    typedef std::deque<PatternList*> pattern_group_t;

    enum SongMode {
	PATTERN_MODE,
	SONG_MODE
    };

    static Song* get_empty_song(Engine* engine);
    static Song* get_default_song(Engine* engine);

    Song( const QString& name, const QString& author, float bpm, float volume );
    ~Song();
	
    /**
       Remove all the notes in the song that play on instrument I.
       The function is real-time safe (it locks the audio data while deleting notes)
    */
    void purge_instrument( Instrument* I );

    void set_volume( float volume );
    float get_volume();
    void set_metronome_volume( float volume );
    float get_metronome_volume();

    void set_mute(bool m);
    bool get_mute();

    void set_resolution(unsigned r);
    unsigned get_resolution();

    void set_bpm(float r);
    float get_bpm();

    void set_modified(bool m);
    bool get_modified();

    void set_name(const QString& name_p);
    const QString& get_name();

    void set_author(const QString& auth);
    const QString& get_author();

    PatternList* get_pattern_list();
    void set_pattern_list( PatternList *pattern_list );

    pattern_group_t* get_pattern_group_vector();
    void set_pattern_group_vector( pattern_group_t* vect );

    static Song* load( Engine* engine, const QString& sFilename );
    bool save( Engine* engine, const QString& sFilename );

    InstrumentList* get_instrument_list();
    void set_instrument_list( InstrumentList *list );

    void set_notes( const QString& notes );
    const QString& get_notes();

    void set_license( const QString& license );
    const QString& get_license();

    const QString& get_filename();
    void set_filename( const QString& filename );

    bool is_loop_enabled();
    void set_loop_enabled( bool enabled );

    float get_humanize_time_value();
    void set_humanize_time_value( float value );

    float get_humanize_velocity_value();
    void set_humanize_velocity_value( float value );

    float get_swing_factor();
    void set_swing_factor( float factor );

    SongMode get_mode();
    void set_mode( SongMode mode );

    /***********************************
     * Methods useful to sequencers.
     ***********************************
     */
    uint32_t song_bar_count();
    uint32_t song_tick_count();
    uint32_t pattern_group_index_for_bar(uint32_t bar);
    uint32_t bar_for_absolute_tick(uint32_t abs_tick);
    uint32_t bar_start_tick(uint32_t bar);
    uint32_t ticks_in_bar(uint32_t bar);

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
    SongPrivate *d;
};

} // namespace Tritium

#endif // TRITIUM_SONG_HPP
