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

#include <hydrogen/TransportPosition.h>
#define BOOST_TEST_MODULE t_TransportPosition
#include <boost/test/unit_test.hpp>

#include <cmath>

using namespace H2Core;

// "BOOST_CHECK( foo )" is too much typing....
#define TX BOOST_CHECK

BOOST_AUTO_TEST_CASE( t_001_defaults )
{
    TransportPosition p;

    // Test the defaults
    TX( p.state == TransportPosition::STOPPED );
    TX( p.new_position == true );
    TX( p.frame == 0 );
    TX( p.bar == 1 );
    TX( p.beat == 1 );
    TX( p.tick == 0 );
    TX( p.bbt_offset == 0 );
    TX( p.bar_start_tick == 0 );
}

BOOST_AUTO_TEST_CASE( t_002_increment )
{
    TransportPosition p;

    p.frame_rate = 48000;
    p.beats_per_bar = 4;
    p.beat_type = 4;
    p.ticks_per_beat = 192;
    p.beats_per_minute = 120.0;
    
    double frames_per_tick = double(p.frame_rate) * (60.0/p.beats_per_minute) / p.ticks_per_beat;
    int k;

    TX( p.frame == 0 );
    double frame = 0.0;
    for( k=1 ; k<p.ticks_per_beat ; ++k ) {
	++p;
	frame += frames_per_tick;
	BOOST_TEST_MESSAGE( "frame = " << frame << "  p.frame = " << p.frame );
	// TODO:  Known bug:  ++ is not changing p.frame.  It's supposed to.
	TX( 1 == p.bar );
	TX( 1 == p.beat );	    
	TX( k == p.tick );
	TX( round(frame) == p.frame );
    }

    TX( p.tick == 191 );
    ++p;
    frame += frames_per_tick;
    TX( 1 == p.bar );
    TX( 2 == p.beat );
    TX( 0 == p.tick );
    TX( round(frame) == p.frame );
}

// TODO: decrement
// TODO: round
// TODO: ceil      --- note that this is known to have a bug.
// TODO: floor
// TODO: etc.... :-)
