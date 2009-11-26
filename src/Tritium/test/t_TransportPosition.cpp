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

#include <Tritium/TransportPosition.hpp>
#include <cmath>

#define THIS_NAMESPACE t_TransportPosition
#include "test_macros.hpp"
#include "test_utils.hpp"

using namespace Tritium;

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
	    // MEMO: p.frames_per_tick() == 125.0

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
	    // MEMO: x.frames_per_tick() == 218.81889588075154
	}

	~Fixture() {}
    };

} // namespace THIS_NAMESPACE

using namespace THIS_NAMESPACE;

TEST_BEGIN( Fixture );

TEST_CASE( 000_defaults )
{
    // Test the defaults
    CK( p.state == TransportPosition::STOPPED );
    CK( p.new_position == true );
    CK( p.frame == 0 );
    CK( p.bar == 1 );
    CK( p.beat == 1 );
    CK( p.tick == 0 );
    CK( p.bbt_offset == 0 );
    CK( p.bar_start_tick == 0 );
}

TEST_CASE( 010_copy_constructor )
{
    TransportPosition y, z;

    CK( y.state == TransportPosition::STOPPED );
    CK( y.new_position == true );
    CK( y.frame == 0 );
    CK( y.bar == 1 );
    CK( y.beat == 1 );
    CK( y.tick == 0 );
    CK( y.bbt_offset == 0 );
    CK( y.bar_start_tick == 0 );

    CK( z.state == TransportPosition::STOPPED );
    CK( z.new_position == true );
    CK( z.frame == 0 );
    CK( z.bar == 1 );
    CK( z.beat == 1 );
    CK( z.tick == 0 );
    CK( z.bbt_offset == 0 );
    CK( z.bar_start_tick == 0 );

    p.frame = 12345;
    p.bar = 21;
    p.beat = 39;
    p.tick = -1;
    y = p;
    CK( 12345 == y.frame );
    CK( 21 == y.bar );
    CK( -1 == y.tick );
    p.tick = 5;
    y.tick = 123;
    CK( 5 == p.tick );
    CK( 123 == y.tick );

    z = x;
    CK( TransportPosition::ROLLING == z.state );
    CK( true == z.new_position );
    CK( 8273901 == z.frame );
    CK( 196123 == z.frame_rate );
    CK( 349 == z.bar );
    CK( 5 == z.beat );
    CK( 18 == z.tick );
    CK( 115 == z.bbt_offset );
    CK( 349 == z.bar_start_tick );
    CK( 7 == z.beats_per_bar );
    CK( 8 == z.beat_type );
    CK( 99 == z.ticks_per_beat );
    CK( 543.2 == z.beats_per_minute );
}

TEST_CASE( 020_frames_per_tick )
{
    CK( p.frames_per_tick() == 125.0 );

    p.frame_rate = 123456;
    p.ticks_per_beat = 48;
    p.beats_per_minute = 33.12;
    CK( round(p.frames_per_tick()) == 4659.0 );

    CK( round(x.frames_per_tick()*100.0) == 21882.0 );
}

TEST_CASE( 030_tick_in_bar )
{
    CK(p.tick_in_bar() == 0);
    p.tick = 191;
    CK(p.tick_in_bar() == 191);
    p.beat = 2;
    CK(p.tick_in_bar() == 383);
    p.bar = 9;
    CK(p.tick_in_bar() == 383);

    CK(x.tick_in_bar() == 414);
}

TEST_CASE( 040_normalize )
{
    TransportPosition a;
    double frame;

    // NOTE: normalize() only changes the frame position if it adjusts
    // bbt_offset.

    a = p;
    a.tick += 192 * 6 + 101;
    a.normalize();
    CK( 2 == a.bar );
    CK( 3 == a.beat );
    CK( 101 == a.tick );
    CK( 192 * 4 == a.bar_start_tick );
    CK( TT_DRIFT(0, a.frame, 1) );

    a.beat -= 4;
    a.normalize();
    CK( 1 == a.bar );
    CK( 3 == a.beat );
    CK( 101 == a.tick );
    CK( 0 == a.bar_start_tick );
    CK( TT_DRIFT(0, a.frame, 2) );

    a.bar += 5;
    a.normalize();
    CK( 6 == a.bar );
    CK( 3 == a.beat );
    CK( 101 == a.tick );
    CK( 0 == a.bar_start_tick ); // If you manually change bar, you also have
                                 // to manually adjust bar_start_tick
    CK( TT_DRIFT(0, a.frame, 3) );

    a = p;
    frame = a.frame;
    a.bbt_offset = 6 * 192 * 125 + 50;
    //frame += a.bbt_offset - 50;
    a.normalize();
    CK( 2 == a.bar );
    CK( 3 == a.beat );
    CK( 0 == a.tick );
    CK( TT_DRIFT(50.0, a.bbt_offset, 1) );
    CK( 192 * 4 == a.bar_start_tick );
    CK( TT_DRIFT(frame, a.frame, 1) );

    a = x;
    frame = a.frame;
    a.tick -= 99 * 12 + 65;
    a.normalize();
    CK( 347 == a.bar );
    CK( 6 == a.beat );
    CK( 52 == a.tick );
    CK( 115 == a.bbt_offset );
    CK( 0 == a.bar_start_tick );
    CK( TT_DRIFT(frame, a.frame, 1) );

    a = x;
    frame = a.frame;
    a.beat += 13;
    a.normalize();
    CK( 351 == a.bar );
    CK( 4 == a.beat );
    CK( 18 == a.tick );
    CK( 115 == a.bbt_offset );
    CK( 349 + 2 * 7 * 99 == a.bar_start_tick );
    CK( TT_DRIFT(frame, a.frame, 1) );

    a = x;
    a.bar_start_tick = 70000;
    a.bar -= 11;
    a.normalize();  // Does not change bar start tick
    CK( 338 == a.bar );
    CK( 5 == a.beat );
    CK( 18 == a.tick );
    CK( 115 == a.bbt_offset );
    CK( 70000 == a.bar_start_tick );

    a = x;                // 349:5.18.115 (70000)  349.59816096502567
    a.bar_start_tick = 70000;
    frame = a.frame;
    a.bar += 3;           // 352:5.18.115 (70000)  352.59816096502567
    a.beat -= 29;         // 348:4.18.115 (67228)  348.45530382216856
    a.tick += 999;        // 349:7.27.115 (67921)  349.89686226372697
    a.bbt_offset += 3114; // 349:7.41.165 (67921)  349.91739754005869
    // frame += 3114.0 - 165.0 + 115.0;
    a.normalize();
    CK( 349 == a.bar );
    CK( 7 == a.beat );
    CK( 41 == a.tick );
    BOOST_MESSAGE( a.bbt_offset );
    CK( fabs(165 - a.bbt_offset) <= 1.0 );
    CK( 67921 == a.bar_start_tick );
    CK( TT_DRIFT(frame, a.frame, 1) );    
}

TEST_CASE( 050_increment )
{
    double frames_per_tick = double(p.frame_rate) * (60.0/p.beats_per_minute) / p.ticks_per_beat;
    int k;

    CK( p.frame == 0 );
    double frame = 0.0;
    for( k=1 ; (unsigned)k<p.ticks_per_beat ; ++k ) {
	++p;
	frame += frames_per_tick;
	CK( 1 == p.bar );
	CK( 1 == p.beat );	    
	CK( k == p.tick );
	CK( TT_DRIFT(frame, p.frame, 1) );
    }

    CK( p.tick == 191 );
    ++p;
    frame += frames_per_tick;
    CK( 1 == p.bar );
    CK( 2 == p.beat );
    CK( 0 == p.tick );
    CK( TT_DRIFT(frame, p.frame, 2) );
    ++p;
    frame += frames_per_tick;
    CK( 1 == p.bar );
    CK( 2 == p.beat );
    CK( 1 == p.tick );
    CK( TT_DRIFT(frame, p.frame, 3) );
    p.bar = 99;
    p.beat = 3;
    ++p;
    frame += frames_per_tick;
    CK( 99 == p.bar );
    CK( 3 == p.beat );
    CK( 2 == p.tick );
    CK( TT_DRIFT(frame, p.frame, 4) );

    // Tests with 'x'
    frame = x.frame;
    frames_per_tick = x.frames_per_tick();
    for( k=x.tick+1 ; (unsigned)k < x.ticks_per_beat ; ++k ) {
	++x;
	frame += frames_per_tick;
	CK( 349 == x.bar );
	CK( 5 == x.beat );
	CK( k == x.tick );
	CK( TT_DRIFT(115.0, x.bbt_offset, k) );
	CK( TT_DRIFT(frame, x.frame, k) );
    }
    CK( x.tick == 98 );
    ++x;
    frame += frames_per_tick;
    CK( 349 == x.bar );
    CK( 6 == x.beat );
    CK( 0 == x.tick );
    CK( TT_DRIFT( 115.0, x.bbt_offset, k+1) );
    CK( TT_DRIFT( frame, x.frame, k+1 ) );

    x.beat = 7;
    x.tick = 98;
    ++x;
    frame += frames_per_tick;
    CK( 350 == x.bar );
    CK( 1 == x.beat );
    CK( 0 == x.tick );
    CK( TT_DRIFT( 115.0, x.bbt_offset, k+2) );
    CK( TT_DRIFT( frame, x.frame, k+2 ) );
    BOOST_MESSAGE( "++ drift = " << (frame - x.frame) );
}

TEST_CASE( 060_decrement )
{
    --p;
    CK( 0 == p.frame );
    CK( 1 == p.bar );
    CK( 1 == p.beat );
    CK( 0 == p.tick );
    CK( 0 == p.bbt_offset );
    CK( 0 == p.bar_start_tick );
    ++p;
    --p;
    CK( 0 == p.frame );
    CK( 1 == p.bar );
    CK( 1 == p.beat );
    CK( 0 == p.tick );
    CK( 0 == p.bbt_offset );
    CK( 0 == p.bar_start_tick );

    p.bar = 5;
    p.beat = 2;
    p.tick = 1;
    --p; --p;
    CK( 0 == p.frame );
    CK( 5 == p.bar );
    CK( 1 == p.beat );
    CK( 191 == p.tick );

    double frame = 0.0;
    double fpt = p.frames_per_tick();
    p.frame = 1000.0;
    p.bar = 5;
    p.beat = 2;
    p.tick = 1;
    frame = p.frame;
    --p; --p;
    frame -= 2.0 * fpt;
    CK( frame >= 0.0 );
    CK( TT_DRIFT(frame, p.frame, 2) );
    CK( 5 == p.bar );
    CK( 1 == p.beat );
    CK( 191 == p.tick );

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
    CK( fabs(drift) < 10.0 );
    CK( p.bar == 99 );
    CK( p.beat == 1 );
    CK( p.tick == 0 );

    // Using the 'x' object
    fpt = x.frames_per_tick();
    frame = x.frame;
    --x;
    frame -= fpt;
    CK( fabs(frame - x.frame) <= 1.0 );

    for( k=2374 ; k > 0 ; --k ) --x;
    frame -= fpt * 2374.0;
    drift = frame - double(x.frame);
    BOOST_MESSAGE( "-- drift @ 2375 = " << drift );
    CK( fabs(drift) < 50.0 );
    CK( 346 == x.bar );
    CK( 2 == x.beat );
    CK( 19 == x.tick );
	
}

TEST_CASE( 070_floor )
{
    p.floor(TransportPosition::TICK);
    CK( 1 == p.bar );
    CK( 1 == p.beat );
    CK( 0 == p.tick );
    CK( 0 == p.frame );
    p.floor(TransportPosition::BEAT);
    CK( 1 == p.bar );
    CK( 1 == p.beat );
    CK( 0 == p.tick );
    CK( 0 == p.frame );
    p.floor(TransportPosition::BAR);
    CK( 1 == p.bar );
    CK( 1 == p.beat );
    CK( 0 == p.tick );
    CK( 0 == p.frame );

    double fpt = p.frames_per_tick();
    p.tick = 1;
    p.bbt_offset = 921;
    p.frame = round(fpt);
    p.floor(TransportPosition::TICK);
    CK( 1 == p.bar );
    CK( 1 == p.beat );
    CK( 8 == p.tick );
    CK( TT_DRIFT(79.0, p.frame, 1) );
    CK( 0 == p.bbt_offset );
    p.tick = 1;
    p.bbt_offset = 921;
    p.frame = round(fpt);
    p.floor(TransportPosition::BEAT);
    CK( 1 == p.bar );
    CK( 1 == p.beat );
    CK( 0 == p.tick );
    CK( 0 == p.frame );
    CK( 0 == p.bbt_offset );
    p.tick = 1;
    p.bbt_offset = 921;
    p.frame = round(fpt);
    p.floor(TransportPosition::BAR);
    CK( 1 == p.bar );
    CK( 1 == p.beat );
    CK( 0 == p.tick );
    CK( 0 == p.frame );
    CK( 0 == p.bbt_offset );

    p.tick = 1;
    p.bbt_offset = 921;
    p.frame = round(fpt/2.0);
    p.floor(TransportPosition::BAR);
    CK( 1 == p.bar );
    CK( 1 == p.beat );
    CK( 0 == p.tick );
    CK( 0 == p.frame );
    CK( 0 == p.bbt_offset );

    TransportPosition tmp = x;
    double lastframe = tmp.frame;
    tmp.floor(TransportPosition::TICK);
    CK( tmp.bar == 349 );
    CK( tmp.beat == 5 );
    CK( tmp.tick == 18 );
    CK( tmp.bbt_offset == 0 );
    lastframe -= 115.0;
    CK( TT_DRIFT(lastframe, tmp.frame, 1) );
    // Repeating when already floored should
    // give the same result.
    tmp.floor(TransportPosition::TICK);
    CK( tmp.bar == 349 );
    CK( tmp.beat == 5 );
    CK( tmp.tick == 18 );
    CK( tmp.bbt_offset == 0 );
    CK( TT_DRIFT(lastframe, tmp.frame, 1) );

    tmp = x;
    tmp.floor(TransportPosition::BEAT);
    CK( tmp.bar == 349 );
    CK( tmp.beat == 5 );
    CK( tmp.tick == 0 );
    CK( fabs(tmp.bbt_offset) <= 0.5 );
    lastframe -= 18.0 * tmp.frames_per_tick();
    CK( TT_DRIFT(lastframe, tmp.frame, 1) );
    // Repeating when already floored should
    // give the same result.
    tmp.floor(TransportPosition::BEAT);
    CK( tmp.bar == 349 );
    CK( tmp.beat == 5 );
    CK( tmp.tick == 0 );
    CK( fabs(tmp.bbt_offset) <= 0.5 );
    CK( TT_DRIFT(lastframe, tmp.frame, 1) );

    tmp = x;
    tmp.floor(TransportPosition::BAR);
    CK( tmp.bar == 349 );
    CK( tmp.beat == 1 );
    CK( tmp.tick == 0 );
    CK( fabs(tmp.bbt_offset) <= 0.5 );
    lastframe = x.frame - 4.0 * 99.0 * tmp.frames_per_tick()
	- x.tick * tmp.frames_per_tick() - x.bbt_offset;
    CK( TT_DRIFT(lastframe, tmp.frame, 1) );
    // Repeating when already floored should
    // give the same result.
    tmp.floor(TransportPosition::BAR);
    CK( tmp.bar == 349 );
    CK( tmp.beat == 1 );
    CK( tmp.tick == 0 );
    CK( fabs(tmp.bbt_offset) <= 0.5 );
    CK( TT_DRIFT(lastframe, tmp.frame, 1) );

}

TEST_CASE( 080_ceil )
{
    p.ceil(TransportPosition::TICK);
    CK( 1 == p.bar );
    CK( 1 == p.beat );
    CK( 0 == p.tick );
    CK( 0 == p.frame );
    p.ceil(TransportPosition::BEAT);
    CK( 1 == p.bar );
    CK( 1 == p.beat );
    CK( 0 == p.tick );
    CK( 0 == p.frame );
    p.ceil(TransportPosition::BAR);
    CK( 1 == p.bar );
    CK( 1 == p.beat );
    CK( 0 == p.tick );
    CK( 0 == p.frame );

    double fpt = p.frames_per_tick();
    double frame;
    p.tick = 1;
    p.bbt_offset = 75;
    p.frame = round(fpt);
    frame = p.frame + fpt - p.bbt_offset;
    p.ceil(TransportPosition::TICK);
    CK( 1 == p.bar );
    CK( 1 == p.beat );
    CK( 2 == p.tick );
    CK( TT_DRIFT(frame, p.frame, 1) );
    CK( fabs(p.bbt_offset) <= 0.5 );
    p.tick = 1;
    p.bbt_offset = 921;
    p.frame = round(fpt);
    p.ceil(TransportPosition::BEAT);
    CK( 1 == p.bar );
    CK( 2 == p.beat );
    CK( 0 == p.tick );
    // frame = [1:1.8.0] + fpt * (192-8)
    //       = 79 + 125.0 * 184 = 23079
    CK( TT_DRIFT(23079.0, p.frame, 2) ); // 2 internal ops.
    CK( fabs(p.bbt_offset) <= 0.5 );
    p.tick = 1;
    p.bbt_offset = 921;
    p.frame = round(fpt);
    p.ceil(TransportPosition::BAR);
    CK( 2 == p.bar );
    CK( 1 == p.beat );
    CK( 0 == p.tick );
    // frame = 125 + [2:1.0.0] - [1:2.1.921]
    //       = 125 + [2:1.0.0] - [1:2.8.46]
    //       = 125 + [1:5.0.0] - [1:2.8.46]
    //       = 125 + fpt* ((2 * 192) + (192-9)) + (125-46)
    //       = 71079.0
    CK( TT_DRIFT(71079.0, p.frame, 2) ); // 2 internal ops
    CK( fabs(p.bbt_offset) <= 0.5 );
    CK( 192 * 4 == p.bar_start_tick );

    p.tick = 1;
    p.bbt_offset = 921;
    p.frame = round(fpt/2.0);
    p.ceil(TransportPosition::BAR);
    CK( 3 == p.bar );
    CK( 1 == p.beat );
    CK( 0 == p.tick );
    // frame = 63 + [3:1.0.0] - [2:1.1.921]
    //       = 63 + [3:1.0.0] - [2:1.8.46]
    //       = 63 + [2:5.0.0] - [2:1.8.46]
    //       = 63 + fpt* ((3 * 192) + (192-9)) + (125-46)
    //       = 95017
    CK( TT_DRIFT(95017.0, p.frame, 2) );
    CK( fabs(p.bbt_offset) <= 0.5 );
    CK( 192 * 4 * 2 == p.bar_start_tick );

    // TODO:  ADD MORE TESTS FOR BAR START TICK

    TransportPosition tmp = x;
    double lastframe = tmp.frame;
    tmp.ceil(TransportPosition::TICK);
    CK( tmp.bar == 349 );
    CK( tmp.beat == 5 );
    CK( tmp.tick == 19 );
    CK( fabs(tmp.bbt_offset) <= 0.5 );
    lastframe += (x.frames_per_tick() - 115.0);
    CK( TT_DRIFT(lastframe, tmp.frame, 1) );
    // Repeating when already ceiled should
    // give the same result.
    tmp.ceil(TransportPosition::TICK);
    CK( tmp.bar == 349 );
    CK( tmp.beat == 5 );
    CK( tmp.tick == 19 );
    CK( fabs(tmp.bbt_offset) <= 0.5 );
    CK( TT_DRIFT(lastframe, tmp.frame, 1) );

    tmp = x;
    lastframe = tmp.frame;
    tmp.ceil(TransportPosition::BEAT);
    CK( tmp.bar == 349 );
    CK( tmp.beat == 6 );
    CK( tmp.tick == 0 );
    CK( fabs(tmp.bbt_offset) <= 0.5 );
    lastframe += (99.0 - 18.0) * tmp.frames_per_tick() - 115.0;
    CK( TT_DRIFT(lastframe, tmp.frame, 1) );
    // Repeating when already ceiled should
    // give the same result.
    tmp.ceil(TransportPosition::BEAT);
    CK( tmp.bar == 349 );
    CK( tmp.beat == 6 );
    CK( tmp.tick == 0 );
    CK( fabs(tmp.bbt_offset) <= 0.5 );
    CK( TT_DRIFT(lastframe, tmp.frame, 1) );

    tmp = x;
    lastframe = tmp.frame;
    lastframe += ((2.0 * 99.0) + (99.0 - 18.0)) * tmp.frames_per_tick()
	- tmp.bbt_offset;
    tmp.ceil(TransportPosition::BAR);
    CK( tmp.bar == 350 );
    CK( tmp.beat == 1 );
    CK( tmp.tick == 0 );
    CK( fabs(tmp.bbt_offset) <= 0.5 );
    CK( TT_DRIFT(lastframe, tmp.frame, 1) );
    // Repeating when already ceiled should
    // give the same result.
    tmp.ceil(TransportPosition::BAR);
    CK( tmp.bar == 350 );
    CK( tmp.beat == 1 );
    CK( tmp.tick == 0 );
    CK( fabs(tmp.bbt_offset) <= 0.5 );
    CK( TT_DRIFT(lastframe, tmp.frame, 1) );
}

TEST_CASE( 090_round )
{
    TransportPosition a;
    double frame;

    a = p;
    a.round(TransportPosition::TICK);
    CK( 0 == a.frame );
    CK( 1 == a.bar );
    CK( 1 == a.beat );
    CK( 0 == a.tick );
    CK( fabs(a.bbt_offset) <= 0.5 );
    a.round(TransportPosition::BEAT);
    CK( 0 == a.frame );
    CK( 1 == a.bar );
    CK( 1 == a.beat );
    CK( 0 == a.tick );
    CK( fabs(a.bbt_offset) <= 0.5 );
    a.round(TransportPosition::BAR);
    CK( 0 == a.frame );
    CK( 1 == a.bar );
    CK( 1 == a.beat );
    CK( 0 == a.tick );
    CK( fabs(a.bbt_offset) <= 0.5 );

    a = p;
    a.bbt_offset = a.frames_per_tick() / 4.0;
    a.round(TransportPosition::TICK);
    CK( 0 == a.frame );
    CK( 1 == a.bar );
    CK( 1 == a.beat );
    CK( 0 == a.tick );
    CK( fabs(a.bbt_offset) <= 0.5 );
    a.bbt_offset = a.frames_per_tick() * 3.0 / 4.0;
    frame = a.frame + round(a.frames_per_tick() / 4.0);
    a.round(TransportPosition::TICK);
    CK( TT_DRIFT( frame, a.frame, 1 ) );
    CK( 1 == a.bar );
    CK( 1 == a.beat );
    CK( 1 == a.tick );
    CK( fabs(a.bbt_offset) <= 0.5 );
    // Re-rounding should not change anything
    a.round(TransportPosition::TICK);
    CK( TT_DRIFT( frame, a.frame, 1 ) );
    CK( 1 == a.bar );
    CK( 1 == a.beat );
    CK( 1 == a.tick );
    CK( fabs(a.bbt_offset) <= 0.5 );

    // Could use more checks near the zero-point,
    // but we'll move on.

    a = x;
    frame = a.frame;
    frame += a.frames_per_tick() - a.bbt_offset;
    a.round(TransportPosition::TICK);
    CK( 349 == a.bar );
    CK( 5 == a.beat );
    CK( 19 == a.tick );
    CK( fabs(a.bbt_offset) <= 0.5 );
    CK( TT_DRIFT( frame, a.frame, 1 ) );

    frame -= a.frames_per_tick() * double(a.tick);
    a.round(TransportPosition::BEAT);
    CK( 349 == a.bar );
    CK( 5 == a.beat );
    CK( 0 == a.tick );
    CK( fabs(a.bbt_offset) <= 0.5 );
    CK( TT_DRIFT( frame, a.frame, 2 ) );

    frame += a.frames_per_tick() * 99.0 * 3.0;
    a.round(TransportPosition::BAR);
    CK( 350 == a.bar );
    CK( 1 == a.beat );
    CK( 0 == a.tick );
    CK( fabs(a.bbt_offset) <= 0.5 );
    CK( TT_DRIFT( frame, a.frame, 3 ) );

    // Could use more checks... but we'll move on.

}

TEST_CASE( 100_operator_plus )
{
    unsigned k;
    TransportPosition a;
    double fpt;
    double frame;

    a = p + (-51);
    CK( 1 == a.bar );
    CK( 1 == a.beat );
    CK( 0 == a.tick );
    CK( fabs(a.bbt_offset) <= 0.5 );
    CK( 0 == a.frame );

    a = p;
    fpt = a.frames_per_tick();
    for( k=0 ; k<191 ; ++k ) {
	a = a + 1;
	CK( 1 == a.bar );
	CK( 1 == a.beat );
	CK( (k+1) == unsigned(a.tick) );
	CK( TT_DRIFT( double(k+1) * fpt, a.frame, k ) );
    }
    CK( 191 == a.tick );
    ++k;
    a = a + 1;
    CK( 1 == a.bar );
    CK( 2 == a.beat );
    CK( 0 == a.tick );
    CK( TT_DRIFT( double(k) * fpt, a.frame, k ) );

    k = 192 * 2 + 30;
    a = p + k;
    CK( 1 == a.bar );
    CK( 3 == a.beat );
    CK( 30 == a.tick );
    CK( TT_DRIFT( double(k) * fpt, a.frame, 1 ) );


    k = 99 * 5 + 64;
    a = x + k;
    frame = x.frame + (99*5 + 64) * x.frames_per_tick();
    CK( 350 == a.bar );
    CK( 3 == a.beat );
    CK( 82 == a.tick );
    CK( TT_DRIFT( 115.0, a.bbt_offset, 1) );
    CK( TT_DRIFT( frame, a.frame, 1 ) );

    a = a + (-k);
    frame = x.frame;
    CK( 349 == a.bar );
    CK( 5 == a.beat );
    CK( 18 == a.tick );
    CK( TT_DRIFT( 115.0, a.bbt_offset, 2) );
    CK( TT_DRIFT( frame, a.frame, 2 ) );
}

TEST_CASE( 110_operator_minus )
{
    unsigned k;
    TransportPosition a;
    double fpt;

    a = p - 51;
    CK( 1 == a.bar );
    CK( 1 == a.beat );
    CK( 0 == a.tick );
    CK( 0 == a.bbt_offset );
    CK( 0 == a.frame );

    a = p;
    a.bar = 2;
    a.beat = 1;
    a.tick = 0;
    a.frame = 2000000;
    fpt = a.frames_per_tick();
    for( k=192 ; k>0 ; --k ) {
	a = a - 1;
	CK( 1 == a.bar );
	CK( 4 == a.beat );
	CK( (k-1) == unsigned(a.tick) );
	CK( TT_DRIFT( 2000000.0 - (double(192-k+1) * fpt), a.frame, (193-k) ) );
    }
    CK( 0 == a.tick );
    a = a - 1;
    k = 193;
    CK( 1 == a.bar );
    CK( 3 == a.beat );
    CK( 191 == a.tick );
    CK( TT_DRIFT( 2000000.0 - (double(k) * fpt), a.frame, k ) );

    k = 99 * 2 + 30;
    a = x - k;
    fpt = x.frames_per_tick();
    CK( 349 == a.bar );
    CK( 2 == a.beat );
    CK( 87 == a.tick );
    CK( TT_DRIFT( 115.0, a.bbt_offset, 1 ) );
    CK( TT_DRIFT( double(x.frame) - (double(k) * fpt), a.frame, 1 ) );

    k = 99 * 5 + 64;
    a = x - k;
    CK( 348 == a.bar );
    CK( 6 == a.beat );
    CK( 53 == a.tick );
    CK( TT_DRIFT( 115.0, a.bbt_offset, 1 ) );
    CK( TT_DRIFT( double(x.frame) - double(k) * fpt, a.frame, 1 ) );

    a = a - (-k);
    CK( 349 == a.bar );
    CK( 5 == a.beat );
    CK( 18 == a.tick );
    CK( TT_DRIFT( 115.0, a.bbt_offset, 2 ) );
    CK( TT_DRIFT( double(x.frame), a.frame, 2 ) );
}

TEST_CASE( 120_operator_plus_equals )
{
    unsigned k;
    TransportPosition a;
    double fpt;

    a = p;
    fpt = a.frames_per_tick();
    for( k=0 ; k<191 ; ++k ) {
	a += 1;
	CK( 1 == a.bar );
	CK( 1 == a.beat );
	CK( k+1 == unsigned(a.tick) );
	CK( TT_DRIFT( double(k+1) * fpt, a.frame, k ) );
    }
    CK( 191 == a.tick );
    ++k;
    a += 1;
    CK( 1 == a.bar );
    CK( 2 == a.beat );
    CK( 0 == a.tick );
    CK( TT_DRIFT( double(k) * fpt, a.frame, k ) );

    k = 192 * 2 + 30;
    a = p;
    a += k;
    CK( 1 == a.bar );
    CK( 3 == a.beat );
    CK( 30 == a.tick );
    CK( TT_DRIFT( double(k) * fpt, a.frame, 1 ) );

    k = 99 * 5 + 64;
    a = x;
    a += k;
    fpt = a.frames_per_tick();
    CK( 350 == a.bar );
    CK( 3 == a.beat );
    CK( 82 == a.tick );
    CK( TT_DRIFT( 115.0, a.bbt_offset, 1 ) );
    CK( TT_DRIFT( double(x.frame) + double(k) * fpt, a.frame, 1 ) );

    a += (-k);
    CK( 349 == a.bar );
    CK( 5 == a.beat );
    CK( 18 == a.tick );
    CK( TT_DRIFT( 115.0, a.bbt_offset, 2 ) );
    CK( TT_DRIFT( double(x.frame), a.frame, 2 ) );
}

TEST_CASE( 130_operator_minus_equals )
{
    unsigned k;
    TransportPosition a;
    double fpt;

    a = p;
    a -= 51;
    CK( 1 == a.bar );
    CK( 1 == a.beat );
    CK( 0 == a.tick );
    CK( 0 == a.bbt_offset );
    CK( 0 == a.frame );

    a = p;
    a.bar = 2;
    a.beat = 1;
    a.tick = 0;
    a.frame = 2000000;
    fpt = a.frames_per_tick();
    for( k=192 ; k>0 ; --k ) {
	a -= 1;
	CK( 1 == a.bar );
	CK( 4 == a.beat );
	CK( (k-1) == unsigned(a.tick) );
	CK( TT_DRIFT( 2000000.0 - (double(192-k+1) * fpt), a.frame, (193-k) ) );
    }
    CK( 0 == a.tick );
    a -= 1;
    k = 193;
    CK( 1 == a.bar );
    CK( 3 == a.beat );
    CK( 191 == a.tick );
    CK( TT_DRIFT( 2000000.0 - (double(k) * fpt), a.frame, k ) );

    k = 99 * 2 + 30;
    a = x;
    a -= k;
    fpt = x.frames_per_tick();
    CK( 349 == a.bar );
    CK( 2 == a.beat );
    CK( 87 == a.tick );
    CK( TT_DRIFT( 115.0, a.bbt_offset, 1 ) );
    CK( TT_DRIFT( double(x.frame) - (double(k) * fpt), a.frame, 1 ) );

    k = 99 * 5 + 64;
    a = x;
    a -= k;
    CK( 348 == a.bar );
    CK( 6 == a.beat );
    CK( 53 == a.tick );
    CK( TT_DRIFT( 115.0, a.bbt_offset, 1 ) );
    CK( TT_DRIFT( double(x.frame) - double(k) * fpt, a.frame, 1 ) );

    a -= (-k);
    CK( 349 == a.bar );
    CK( 5 == a.beat );
    CK( 18 == a.tick );
    CK( TT_DRIFT( 115.0, a.bbt_offset, 2 ) );
    CK( TT_DRIFT( double(x.frame), a.frame, 2 ) );
}

TEST_END()
