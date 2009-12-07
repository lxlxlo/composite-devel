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
 * This test was originally to test the methods in the songhelpers.cpp
 * file.  However, these have now been merged into class Song.  Since
 * these tests are written and work, they are retained with the
 * t_songhelpers.cpp name.
 */
#include <Tritium/Engine.hpp>
#include <Tritium/Song.hpp>
#include <Tritium/Logger.hpp>
#include <Tritium/Preferences.hpp>

#define THIS_NAMESPACE t_songhelpers
#include "test_macros.hpp"

using namespace Tritium;

namespace THIS_NAMESPACE
{
    const char song_file_name[] = T_SONGHELPERS_H2SONG;

    struct Fixture
    {
	Song* s;

	Fixture() : s(0) {
	    Logger::create_instance();
	    Preferences *prefs = new Preferences();
	    Engine::create_instance(prefs);
	    BOOST_MESSAGE(song_file_name);
	    s = Song::load(song_file_name);
	    BOOST_REQUIRE( s != 0 );
	}

	~Fixture() {
	    delete s;
	    delete Engine::get_instance();
	    delete Logger::get_instance();
	}
    };

} // namespace THIS_NAMESPACE

TEST_BEGIN( Fixture );

TEST_CASE( 010_defaults )
{
    BOOST_MESSAGE( s->get_name().toStdString() );
    CK( s->get_name() == QString("Jazzy") );
    CK( s->song_bar_count() == 8 );
    CK( s->song_tick_count() == 1536 );
    CK( s->bar_for_absolute_tick( 0 ) == 1 );
    CK( s->bar_start_tick( 1 ) == 0 );
    CK( s->ticks_in_bar( 1 ) == 192 );
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
