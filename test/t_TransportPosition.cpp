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

#define THIS_NAMESPACE t_TransportPosition
#define THIS(x) t_TransportPosition##_##x

using namespace H2Core;

// "BOOST_CHECK( foo )" is too much typing....
#define TX BOOST_CHECK

namespace THIS_NAMESPACE
{
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
} // namespace THIS_NAMESPACE

using namespace THIS_NAMESPACE;

BOOST_FIXTURE_TEST_SUITE( THIS_NAMESPACE, Fixture );

BOOST_AUTO_TEST_CASE( THIS(000_defaults) )
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

BOOST_AUTO_TEST_CASE( THIS(001_copy_constructor) )
{
    TransportPosition y, z;

    TX( y.state == TransportPosition::STOPPED );
    TX( y.new_position == true );
    TX( y.frame == 0 );
    TX( y.bar == 1 );
    TX( y.beat == 1 );
    TX( y.tick == 0 );
    TX( y.bbt_offset == 0 );
    TX( y.bar_start_tick == 0 );

    TX( z.state == TransportPosition::STOPPED );
    TX( z.new_position == true );
    TX( z.frame == 0 );
    TX( z.bar == 1 );
    TX( z.beat == 1 );
    TX( z.tick == 0 );
    TX( z.bbt_offset == 0 );
    TX( z.bar_start_tick == 0 );

    p.frame = 12345;
    p.bar = 21;
    p.beat = 39;
    p.tick = -1;
    y = p;
    TX( 12345 == y.frame );
    TX( 21 == y.bar );
    TX( -1 == y.tick );
    p.tick = 5;
    y.tick = 123;
    TX( 5 == p.tick );
    TX( 123 == y.tick );

    z = x;
    TX( TransportPosition::ROLLING == z.state );
    TX( true == z.new_position );
    TX( 8273901 == z.frame );
    TX( 196123 == z.frame_rate );
    TX( 349 == z.bar );
    TX( 5 == z.beat );
    TX( 18 == z.tick );
    TX( 115 == z.bbt_offset );
    TX( 349 == z.bar_start_tick );
    TX( 7 == z.beats_per_bar );
    TX( 8 == z.beat_type );
    TX( 99 == z.ticks_per_beat );
    TX( 543.2 == z.beats_per_minute );
}

BOOST_AUTO_TEST_CASE( THIS(002_frames_per_tick) )
{
    TX( p.frames_per_tick() == 125.0 );

    p.frame_rate = 123456;
    p.ticks_per_beat = 48;
    p.beats_per_minute = 33.12;
    TX( round(p.frames_per_tick()) == 4659.0 );

    TX( round(x.frames_per_tick()*100.0) == 21882.0 );
}

BOOST_AUTO_TEST_CASE( THIS(003_tick_in_bar) )
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

BOOST_AUTO_TEST_CASE( THIS(004_increment) )
{
    double frames_per_tick = double(p.frame_rate) * (60.0/p.beats_per_minute) / p.ticks_per_beat;
    int k;

    TX( p.frame == 0 );
    double frame = 0.0;
    for( k=1 ; (unsigned)k<p.ticks_per_beat ; ++k ) {
	++p;
	frame += frames_per_tick;
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
    ++p;
    frame += frames_per_tick;
    TX( 1 == p.bar );
    TX( 2 == p.beat );
    TX( 1 == p.tick );
    TX( round(frame) == p.frame );
    p.bar = 99;
    p.beat = 3;
    ++p;
    frame += frames_per_tick;
    TX( 99 == p.bar );
    TX( 3 == p.beat );
    TX( 2 == p.tick );
    TX( round(frame) == p.frame );

    // Tests with 'x'
    frame = x.frame;
    frames_per_tick = x.frames_per_tick();
    double max_drift = 6.0;
    for( k=x.tick+1 ; (unsigned)k < x.ticks_per_beat ; ++k ) {
	++x;
	frame += frames_per_tick;
	TX( 349 == x.bar );
	TX( 5 == x.beat );
	TX( k == x.tick );
	TX( 115 == x.bbt_offset );
	TX( fabs( frame - double(x.frame) ) < max_drift );
    }
    TX( x.tick == 98 );
    ++x;
    frame += frames_per_tick;
    TX( 349 == x.bar );
    TX( 6 == x.beat );
    TX( 0 == x.tick );
    TX( 115 == x.bbt_offset );
    TX( fabs( frame - double(x.frame) ) < max_drift );

    x.beat = 7;
    x.tick = 98;
    ++x;
    frame += frames_per_tick;
    TX( 350 == x.bar );
    TX( 1 == x.beat );
    TX( 0 == x.tick );
    TX( 115 == x.bbt_offset );
    TX( fabs( frame - double(x.frame) ) < max_drift );  // this is also because of roundoff error.
    BOOST_MESSAGE( "++ drift = " << (frame - x.frame) );
}

BOOST_AUTO_TEST_CASE( THIS(005_decrement) )
{
    --p;
    TX( 0 == p.frame );
    TX( 1 == p.bar );
    TX( 1 == p.beat );
    TX( 0 == p.tick );
    TX( 0 == p.bbt_offset );
    TX( 0 == p.bar_start_tick );
    ++p;
    --p;
    TX( 0 == p.frame );
    TX( 1 == p.bar );
    TX( 1 == p.beat );
    TX( 0 == p.tick );
    TX( 0 == p.bbt_offset );
    TX( 0 == p.bar_start_tick );

    p.bar = 5;
    p.beat = 2;
    p.tick = 1;
    --p; --p;
    TX( 0 == p.frame );
    TX( 5 == p.bar );
    TX( 1 == p.beat );
    TX( 191 == p.tick );

    double frame = 0.0;
    double fpt = p.frames_per_tick();
    p.frame = 1000.0;
    p.bar = 5;
    p.beat = 2;
    p.tick = 1;
    frame = p.frame;
    --p; --p;
    frame -= 2.0 * fpt;
    TX( frame >= 0.0 );
    TX( round(frame) == p.frame );
    TX( 5 == p.bar );
    TX( 1 == p.beat );
    TX( 191 == p.tick );

    frame = 100000.0;
    p.frame = frame;
    p.bar = 100;
    p.beat = 1;
    p.tick = 0;
    int k = p.ticks_per_beat * p.beats_per_bar;
    frame -= fpt * k;
    while( k > 0 ) {
	--k; --p;
    }
    double drift = fabs( frame - double(p.frame) );
    BOOST_MESSAGE( "-- drift @ 768 = " << drift );
    TX( fabs(drift) < 10.0 );
    TX( p.bar == 99 );
    TX( p.beat == 1 );
    TX( p.tick == 0 );

    // Using the 'x' object
    fpt = x.frames_per_tick();
    frame = x.frame;
    --x;
    frame -= fpt;
    TX( fabs(frame - x.frame) <= 1.0 );

    for( k=2374 ; k > 0 ; --k ) --x;
    frame -= fpt * 2374.0;
    drift = frame - double(x.frame);
    BOOST_MESSAGE( "-- drift @ 2375 = " << drift );
    TX( fabs(drift) < 50.0 );
    TX( 346 == x.bar );
    TX( 2 == x.beat );
    TX( 19 == x.tick );
	
}

BOOST_AUTO_TEST_CASE( THIS(006_floor) )
{
    p.floor(TransportPosition::TICK);
    TX( 1 == p.bar );
    TX( 1 == p.beat );
    TX( 0 == p.tick );
    TX( 0 == p.frame );
    p.floor(TransportPosition::BEAT);
    TX( 1 == p.bar );
    TX( 1 == p.beat );
    TX( 0 == p.tick );
    TX( 0 == p.frame );
    p.floor(TransportPosition::BAR);
    TX( 1 == p.bar );
    TX( 1 == p.beat );
    TX( 0 == p.tick );
    TX( 0 == p.frame );

    double fpt = p.frames_per_tick();
    p.tick = 1;
    p.bbt_offset = 921;
    p.frame = round(fpt);
    p.floor(TransportPosition::TICK);
    TX( 1 == p.bar );
    TX( 1 == p.beat );
    TX( 1 == p.tick );
    TX( 0 == p.frame );
    TX( 0 == p.bbt_offset );
    p.tick = 1;
    p.bbt_offset = 921;
    p.frame = round(fpt);
    p.floor(TransportPosition::BEAT);
    TX( 1 == p.bar );
    TX( 1 == p.beat );
    TX( 0 == p.tick );
    TX( 0 == p.frame );
    TX( 0 == p.bbt_offset );
    p.tick = 1;
    p.bbt_offset = 921;
    p.frame = round(fpt);
    p.floor(TransportPosition::BAR);
    TX( 1 == p.bar );
    TX( 1 == p.beat );
    TX( 0 == p.tick );
    TX( 0 == p.frame );
    TX( 0 == p.bbt_offset );

    p.tick = 1;
    p.bbt_offset = 921;
    p.frame = round(fpt/2.0);
    p.floor(TransportPosition::BAR);
    TX( 1 == p.bar );
    TX( 1 == p.beat );
    TX( 0 == p.tick );
    TX( 0 == p.frame );
    TX( 0 == p.bbt_offset );

    TransportPosition tmp = x;
    double lastframe = tmp.frame;
    tmp.floor(TransportPosition::TICK);
    TX( tmp.bar == 349 );
    TX( tmp.beat == 5 );
    TX( tmp.tick == 18 );
    TX( tmp.bbt_offset == 0 );
    lastframe -= 115.0;
    TX( tmp.frame == round(lastframe) );
    // Repeating when already floored should
    // give the same result.
    tmp.floor(TransportPosition::TICK);
    TX( tmp.bar == 349 );
    TX( tmp.beat == 5 );
    TX( tmp.tick == 18 );
    TX( tmp.bbt_offset == 0 );
    TX( tmp.frame == round(lastframe) );

    tmp = x;
    tmp.floor(TransportPosition::BEAT);
    TX( tmp.bar == 349 );
    TX( tmp.beat == 5 );
    TX( tmp.tick == 0 );
    TX( tmp.bbt_offset == 0 );
    lastframe -= 18.0 * tmp.frames_per_tick();
    TX( tmp.frame == round(lastframe) );
    // Repeating when already floored should
    // give the same result.
    tmp.floor(TransportPosition::BEAT);
    TX( tmp.bar == 349 );
    TX( tmp.beat == 5 );
    TX( tmp.tick == 0 );
    TX( tmp.bbt_offset == 0 );
    TX( tmp.frame == round(lastframe) );

    tmp = x;
    tmp.floor(TransportPosition::BAR);
    TX( tmp.bar == 349 );
    TX( tmp.beat == 1 );
    TX( tmp.tick == 0 );
    TX( tmp.bbt_offset == 0 );
    lastframe -= 4.0 * 99.0 * tmp.frames_per_tick();
    TX( tmp.frame == round(lastframe) );
    // Repeating when already floored should
    // give the same result.
    tmp.floor(TransportPosition::BAR);
    TX( tmp.bar == 349 );
    TX( tmp.beat == 1 );
    TX( tmp.tick == 0 );
    TX( tmp.bbt_offset == 0 );
    TX( tmp.frame == round(lastframe) );

}

BOOST_AUTO_TEST_CASE( THIS(007_ceil) )
{
    TX( false );  // Need to implement test
}

BOOST_AUTO_TEST_CASE( THIS(008_round) )
{
    TX( false );  // Need to implement test
}

BOOST_AUTO_TEST_CASE( THIS(009_operator_plus) )
{
    TX( false );  // Need to implement test
}

BOOST_AUTO_TEST_CASE( THIS(010_operator_minus) )
{
    TX( false );  // Need to implement test
}

BOOST_AUTO_TEST_CASE( THIS(010_operator_plus_equals) )
{
    TX( false );  // Need to implement test
}

BOOST_AUTO_TEST_CASE( THIS(011_operator_minus_equals) )
{
    TX( false );  // Need to implement test
}

BOOST_AUTO_TEST_SUITE_END()
