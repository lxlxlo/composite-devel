/*
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
 * This test verifies the workings of Tritium::Song.
 */
#include <Tritium/Engine.hpp>
#include <Tritium/Song.hpp>
#include <Tritium/Logger.hpp>
#include <Tritium/Preferences.hpp>
#include <Tritium/Pattern.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/InstrumentList.hpp>

#define THIS_NAMESPACE t_Song
#include "test_macros.hpp"
#include "test_config.hpp"

using namespace Tritium;

namespace THIS_NAMESPACE
{
    const char song_file_name[] = TEST_DATA_DIR "/t_Song.h2song";

    struct Fixture
    {
	Song* s;
	Engine* engine;

	Fixture() : s(0) {
	    Logger::create_instance();
	    Preferences *prefs = new Preferences();
	    engine = new Engine(prefs);
	    BOOST_MESSAGE(song_file_name);
	    s = Song::load(engine, song_file_name);
	    BOOST_REQUIRE( s != 0 );
	}

	~Fixture() {
	    delete s;
	    delete engine;
	    delete Logger::get_instance();
	}
    };

} // namespace THIS_NAMESPACE

TEST_BEGIN( Fixture );

TEST_CASE( 010_defaults )
{
    /*********************************
     * Using constructor
     *********************************
     */
    std::auto_ptr<Song> z( new Song("Empty", "Scripty", 134.9f, 0.8f ) );

    // Metadata
    CK( z->get_name() == "Empty" );
    CK( z->get_volume() == 0.8f );
    CK( z->get_metronome_volume() == 0.5f );
    CK( z->get_mute() == false );
    CK( z->get_resolution() == 48 );
    CK( z->get_bpm() == 134.9f );
    CK( z->get_modified() == false );
    CK( z->get_author() == "Scripty" );
    CK( z->get_license() == "" );
    CK( z->get_filename() == "" );

    // Pattern and Instrument
    CK( z->get_pattern_list()->get_size() == 0 );
    CK( z->get_pattern_group_vector()->size() == 0 );
    CK( z->get_instrument_list()->get_size() == 0 );
    CK( z->get_notes() == "" );
    CK( z->is_loop_enabled() == false );
    CK( z->get_humanize_time_value() == 0.0 );
    CK( z->get_humanize_velocity_value() == 0.0 );
    CK( z->get_swing_factor() == 0 );
    CK( z->get_mode() == Song::PATTERN_MODE );

    // "songhelper" methods
    CK( z->song_bar_count() == 0 );
    CK( z->song_tick_count() == 0 );
    CK( z->pattern_group_index_for_bar(1) == -1 );
    CK( z->bar_for_absolute_tick(0) == -1 );
    CK( z->bar_start_tick(1) == -1 );
    CK( z->ticks_in_bar(1) == -1 );

    // Pattern Mode methods
    CK( z->get_pattern_mode_type() == Song::SINGLE );

    /*********************************
     * Using static method
     *********************************
     */
    std::auto_ptr<Song> w( Song::get_default_song(engine) );

    // Metadata
    CK( w->get_name() == "empty" );
    CK( w->get_volume() == 1.0 );
    CK( w->get_metronome_volume() == 0.5 );
    CK( w->get_mute() == false );
    CK( w->get_resolution() == 48 );
    CK( w->get_bpm() == 120.0 );
    CK( w->get_modified() == false );
    CK( w->get_author() == "Tritium" );
    CK( w->get_license() == "" );
    CK( w->get_filename() == "" );

    // Pattern and Instrument
    CK( w->get_pattern_list() == 0 );
    CK( w->get_pattern_group_vector() == 0 );
    CK( w->get_instrument_list() == 0 );
    CK( w->get_notes() == "..." );
    CK( w->is_loop_enabled() == false );
    CK( w->get_humanize_time_value() == 0.0 );
    CK( w->get_humanize_velocity_value() == 0.0 );
    CK( w->get_swing_factor() == 0 );
    CK( w->get_mode() == Song::PATTERN_MODE );

    // "songhelper" methods
    CK( w->song_bar_count() == 0 );
    CK( w->song_tick_count() == 0 );
    CK( w->pattern_group_index_for_bar(1) == -1 );
    CK( w->bar_for_absolute_tick(0) == -1 );
    CK( w->bar_start_tick(1) == -1 );
    CK( w->ticks_in_bar(1) == -1 );

    // Pattern Mode methods
    CK( w->get_pattern_mode_type() == Song::SINGLE );
    Tritium::PatternList pl;
    w->get_playing_patterns(pl);
    CK( pl.get_size() == 0 );

}

TEST_CASE( 015_song_loading )
{
    BOOST_MESSAGE( s->get_name().toStdString() );

    // Metadata
    CK( s->get_name() == "Jazzy" );
    CK( s->get_volume() == 0.5 );
    CK( s->get_metronome_volume() == 0.5 );
    CK( s->get_mute() == false );
    CK( s->get_resolution() == 48 );
    CK( s->get_bpm() == 100.0f );
    CK( s->get_modified() == false );
    CK( s->get_author() == "Emiliano Grilli" );
    CK( s->get_license() == "" );
    CK( s->get_filename() == "" );

    // Pattern and Instrument
    CK( s->get_pattern_list()->get_size() == 3 );
    CK( s->get_pattern_group_vector()->size() == 8 );
    CK( s->get_instrument_list()->get_size() == 32 );
    CK( s->get_notes() == "Jazzy..." );
    CK( s->is_loop_enabled() == true );
    CK( s->get_humanize_time_value() == 0.0 );
    CK( s->get_humanize_velocity_value() == 0.23f );
    CK( s->get_swing_factor() == 0.44f );
    CK( s->get_mode() == Song::SONG_MODE );

    // "songhelper" methods
    CK( s->song_bar_count() == 8 );
    CK( s->song_tick_count() == 1536 );
    CK( s->pattern_group_index_for_bar(1) == 0 );
    CK( s->bar_for_absolute_tick(0) == 1 );
    CK( s->bar_start_tick(1) == 0 );
    CK( s->ticks_in_bar(1) == 192 );

    // Pattern Mode methods
    CK( s->get_pattern_mode_type() == Song::SINGLE );
    Tritium::PatternList pl;
    s->get_playing_patterns(pl);
    CK( pl.get_size() == 1 );

    // Basic instrument loading.
    // (Detailed instrument loading should be done in a
    // a different test.  This is t_Song.)
    Tritium::InstrumentList *list;
    Tritium::Instrument *inst;
    const char* names[] = {
	"Kick", "Stick", "Snare Jazz", "Hand Clap", "Snare Rock", "Tom Low",
	"Closed HH", "Tom Mid", "Pedal HH", "Tom Hi", "Open HH", "Cowbell",
	"Ride Jazz", "Crash", "Ride Rock", "Crash Jazz", "17", "18", "19",
	"20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30",
	"31", "32"
    };
    int k;

    list = s->get_instrument_list();
    CK( list->get_size() == 32 );
    for(k=0 ; k<list->get_size() ; ++k) {
	inst = list->get(k);
	CK( inst != 0 );
	CK( inst->get_name() == names[k] );
    }

}

TEST_CASE( 020_pattern_group_index_for_bar )
{
    for( uint32_t k = 0 ; k < 8 ; ++k ) {
	CK( s->pattern_group_index_for_bar(k+1) == k );
    }
}

TEST_CASE( 030_bar_for_absolute_tick )
{
    uint32_t bar, real_bar;
    for( uint32_t k = 0 ; k < 1536 ; ++k ) {
	real_bar = (k/192) + 1;
	bar = s->bar_for_absolute_tick(k);
	CK( bar == real_bar );
    }
}

TEST_CASE( 040_bar_start_tick )
{
    uint32_t bst, real_bst;
    for( uint32_t k = 1 ; k <= 8 ; ++k ) {
	real_bst = 192 * (k-1);
	bst = s->bar_start_tick(k);
	CK( bst == real_bst );
    }
}

TEST_CASE( 050_ticks_in_bar )
{
    uint32_t ticks, real_ticks;
    for( uint32_t k = 1 ; k <= 8 ; ++k ) {
	real_ticks = 192;
	ticks = s->ticks_in_bar(k);
	CK( ticks == real_ticks );
    }
}

TEST_END()
