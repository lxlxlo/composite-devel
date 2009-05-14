/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <hydrogen/Song.h>
#include <transport/songhelpers.h>

#define THIS_NAMESPACE t_songhelpers
#include "test_macros.h"

using namespace H2Core;

namespace THIS_NAMESPACE
{

    struct Fixture
    {
	Song* s;

	Fixture() : s(0) {
	    // Path relative to location of t_Hydrogen executable
	    s = Song::load("test/t_songhelpers.h2song");
	    BOOST_REQUIRE( s != 0 );
	}

	~Fixture() {
	    delete s;
	}
    };

} // namespace THIS_NAMESPACE

TEST_BEGIN( Fixture );

TEST_CASE( 010_defaults )
{
    BOOST_MESSAGE( s->__name.toStdString() );
    CK( s->__name == QString("Jazzy") );
    CK( song_bar_count( s ) == 8 );
    CK( song_tick_count( s ) == 1536 );
    CK( bar_for_absolute_tick( s, 0 ) == 1 );
    CK( bar_start_tick( s, 1 ) == 0 );
    CK( ticks_in_bar( s, 1 ) == 192 );
}

TEST_CASE( 020_pattern_group_index_for_bar )
{
    for( uint32_t k = 0 ; k < 8 ; ++k ) {
	CK( pattern_group_index_for_bar(s, k+1) == k );
    }
}

TEST_CASE( 030_bar_for_absolute_tick )
{
    uint32_t bar, real_bar;
    for( uint32_t k = 0 ; k < 1536 ; ++k ) {
	real_bar = (k/192) + 1;
	bar = bar_for_absolute_tick(s, k);
	CK( bar == real_bar );
    }
}

TEST_CASE( 040_bar_start_tick )
{
    uint32_t bst, real_bst;
    for( uint32_t k = 1 ; k <= 8 ; ++k ) {
	real_bst = 192 * (k-1);
	bst = bar_start_tick(s, k);
	CK( bst == real_bst );
    }
}

TEST_CASE( 050_ticks_in_bar )
{
    uint32_t ticks, real_ticks;
    for( uint32_t k = 1 ; k <= 8 ; ++k ) {
	real_ticks = 192;
	ticks = ticks_in_bar(s, k);
	CK( ticks == real_ticks );
    }
}

TEST_END()
