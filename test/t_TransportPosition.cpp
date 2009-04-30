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
#include <boost/test/unit_test.hpp>

#include <cmath>

using namespace H2Core;

// "BOOST_CHECK( foo )" is too much typing....
#define TX BOOST_CHECK

struct Fixture
{
    TransportPosition p;  // This is the "normal" one.
    TransportPosition x;  // This one has an odd setup.

    Fixture() : p() {
	p.frame_rate = 48000;
	p.beats_per_bar = 4;
	p.beat_type = 4;
	p.ticks_per_beat = 192;
	p.beats_per_minute = 120.0;

	// x init.
	x.state = TransportPosition::ROLLING;
	x.new_position = true;
	x.frame = 8273901;
	x.frame_rate = 196123;
	x.bar = 349;
	x.beat = 5;
	x.tick = 18;
	x.bbt_offset = 115;
	x.bar_start_tick = 349;
	x.beats_per_bar = 7;
	x.beat_type = 8;
	x.ticks_per_beat = 99;
	x.beats_per_minute = 543.2;
	
    }

    ~Fixture() {}
};

BOOST_FIXTURE_TEST_SUITE( t_TransportPosition, Fixture );

BOOST_AUTO_TEST_CASE( t_001_defaults )
{
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

BOOST_AUTO_TEST_CASE( t_002_frames_per_tick )
{
    TX( p.frames_per_tick() == 125.0 );

    p.frame_rate = 123456;
    p.ticks_per_beat = 48;
    p.beats_per_minute = 33.12;
    TX( round(p.frames_per_tick()) == 4659.0 );

    TX( round(x.frames_per_tick()*100.0) == 21882.0 );
}

BOOST_AUTO_TEST_CASE( t_003_tick_in_bar )
{
    TX(p.tick_in_bar() == 0);
    p.tick = 191;
    TX(p.tick_in_bar() == 191);
    p.beat = 2;
    TX(p.tick_in_bar() == 383);
    p.bar = 9;
    TX(p.tick_in_bar() == 383);

    TX(x.tick_in_bar() == 414);
}

BOOST_AUTO_TEST_CASE( t_004_increment )
{
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

    TX( false ); // Do some tests with the 'x' object.
}

BOOST_AUTO_TEST_CASE( t_005_decrement )
{
    TX( false );  // Need to implement test
}

BOOST_AUTO_TEST_CASE( t_006_round )
{
    TX( false );  // Need to implement test
}

BOOST_AUTO_TEST_CASE( t_007_floor )
{
    TX( false );  // Need to implement test
}

BOOST_AUTO_TEST_CASE( t_008_ceil )
{
    TX( false );  // Need to implement test
}

BOOST_AUTO_TEST_CASE( t_009_operator_plus )
{
    TX( false );  // Need to implement test
}

BOOST_AUTO_TEST_CASE( t_010_operator_minus )
{
    TX( false );  // Need to implement test
}

BOOST_AUTO_TEST_CASE( t_010_operator_plus_equals )
{
    TX( false );  // Need to implement test
}

BOOST_AUTO_TEST_CASE( t_011_operator_minus_equals )
{
    TX( false );  // Need to implement test
}

BOOST_AUTO_TEST_CASE( t_012_copy_constructor )
{
    TX( false );  // Need to implement test
}

BOOST_AUTO_TEST_SUITE_END()
