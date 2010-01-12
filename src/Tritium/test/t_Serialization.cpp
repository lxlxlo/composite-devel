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

/**
 * t_TestTemplate.cpp
 *
 * This is just the template for a test.  If the template is not
 * executable, it will not be kept up-to-date.
 */

#include <Tritium/Serialization.hpp>
#include <Tritium/ObjectBundle.hpp>
#include <Tritium/memory.hpp>
#include <Tritium/Logger.hpp>
#include <Tritium/Engine.hpp>
#include <Tritium/Song.hpp>
#include <Tritium/Pattern.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/InstrumentList.hpp>
#include <Tritium/Preferences.hpp>
#include <Tritium/fx/Effects.hpp>
#include <Tritium/fx/LadspaFX.hpp>
#include <cstdio>
#include <deque>
#include <sys/stat.h> // for mkdir()
#include <sys/types.h> // for mkdir()

// CHANGE THIS TO MATCH YOUR FILE:
#define THIS_NAMESPACE t_Serialization
#include "test_macros.hpp"
#include "test_config.hpp"

using namespace Tritium;
using namespace Tritium::Serialization;

namespace THIS_NAMESPACE
{
    class SyncBundle : public ObjectBundle
    {
    public:
	bool done;

	SyncBundle() : done(false) {}
	void operator()() { done = true; }
    };

    const char temp_dir[] = "t_Serialization_tmp";
    const char song_file_name[] = TEST_DATA_DIR "/t_Serialization.h2song";

    struct Fixture
    {
	// SETUP AND TEARDOWN OBJECTS FOR YOUR TESTS.
	T<Serializer>::auto_ptr s;
	T<Engine>::auto_ptr engine;
	Fixture() : s(0) {
	    Logger::create_instance();
	    T<Preferences>::shared_ptr prefs(new Preferences );
	    engine.reset( new Engine(prefs) );
	    s.reset( Serializer::create_standalone(engine.get()) );
	    int rv;
	    rv = ::mkdir(temp_dir, 0700);
	    BOOST_REQUIRE( rv == 0 );
	}
	~Fixture() {
	    int rv;
	    rv = ::remove(temp_dir);
	    BOOST_REQUIRE( rv == 0 );
	    s.reset();
	    engine.reset();
	    delete Logger::get_instance();
	}
    };

} // namespace THIS_NAMESPACE

TEST_BEGIN( Fixture );

TEST_CASE( 000_load_invalid_file_name )
{
    const char fn[] = TEST_DATA_DIR "/really_unlikely_filename.txt";
    SyncBundle bdl;

    s->load_file(fn, bdl, engine.get());

    while( ! bdl.done ) {
	sleep(1);
    }

    CK( bdl.error );
    CK( bdl.empty() );
}

TEST_CASE( 010_load_song_check_song )
{
    SyncBundle bdl;

    s->load_file(song_file_name, bdl, engine.get());

    while( ! bdl.done ) {
	sleep(1);
    }

    BOOST_REQUIRE( ! bdl.error );

    // Sort out all the components:
    std::deque< T<Song>::shared_ptr > songs;
    std::deque< T<Pattern>::shared_ptr > patterns;
    std::deque< T<Instrument>::shared_ptr > instruments;
    std::deque< T<LadspaFX>::shared_ptr > effects;

    while( ! bdl.empty() ) {
	switch(bdl.peek_type()) {
	case ObjectItem::Song_t:
	    songs.push_back( bdl.pop<Song>() );
	    break;
	case ObjectItem::Pattern_t:
	    patterns.push_back( bdl.pop<Pattern>() );
	    break;
	case ObjectItem::Instrument_t:
	    instruments.push_back( bdl.pop<Instrument>() );
	    break;
	case ObjectItem::LadspaFX_t:
	    effects.push_back( bdl.pop<LadspaFX>() );
	    break;
	default:
	    CK(false); // should not reach this.
	}
    }

    CK( songs.size() == 1 );
    CK( patterns.size() == 0 );
    CK( instruments.size() == 32 );
    CK( effects.size() == 0 );
    
    /********************************************
     * SONG Object
     ********************************************
     */
    T<Song>::shared_ptr s = songs.front();

    // Metadata
    CK( s->get_name() == "Jazzy" );
    CK( s->get_volume() == 0.5 );
    CK( s->get_metronome_volume() == 0.5 );
    CK( s->get_mute() == false );
    CK( s->get_resolution() == 48 );
    CK( s->get_bpm() == 100.0f );
    CK( s->get_modified() == false );
    CK( s->get_author() == "Emiliano Grilli" );
    CK( s->get_license() == "Unknown license" );
    CK( s->get_filename() == song_file_name );

    // Pattern and Instrument
    CK( s->get_pattern_list()->get_size() == 3 );
    CK( s->get_pattern_group_vector()->size() == 8 );
    // Serializer doesn't load instruments into Song,
    // since that's about to be wrong.
    CK( s->get_instrument_list()->get_size() == 0 );
    CK( s->get_notes() == "Jazzy..." );
    CK( s->is_loop_enabled() == true );
    CK( s->get_humanize_time_value() == 0.23f );
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
    CK( pl.get_size() == 0 );

    /********************************************
     * Instrument Objects
     ********************************************
     */
    BOOST_ERROR("Need to write more tests");
}

TEST_CASE( 020_save_song )
{
}

TEST_END()
