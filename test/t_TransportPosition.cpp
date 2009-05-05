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

/**********************************************************************
 * NOTE ON COMPARING FRAMES AND FRAME ADJUSTMENT
 * =============================================
 *
 * Suppose you are summing x(n+1) = x(n) + e where x(i) and e are real
 * numbers, and x(0) is known.
 * 
 * If you calculate like this: x(n+1) = round(x(n) + e) you will
 * consistantly drift away from the actual answer.  However, this is
 * common to do in frame adjustment calculations where the tempo does
 * not align with the frame rate.  The maximum error in this situation
 * is 0.5 * n.
 *
 * To mitigate this error (without keeping track of decimal
 * bbt_offsets), TransportPosition uses dithering.  A random number
 * between [-.5, .5) is added to the number before rounding.  So:
 *
 * x(n+1) = round( x(n) + e + dither() )
 *
 * Theoretically, all of the dither()'s should sum up to zero and you
 * would (usually) have no error over large 'n'.  However, it's not
 * exactly the case.  In order to have tests that always pass, it
 * would be nice to know what the tolerance is for error when
 * comparing expected frame values to calculated frame values.
 *
 * To find a boundary for the error introduced by dithering (as
 * opposed to the error introduced by rounding), I ran about 5000 sets
 * with e between 0 and 11, and max n = 100000... taking the maximum
 * errors for various slots.  (E.g. max error for n = [90, 100])
 *
 * I hand-fit [1] a couple of equations that always bounded the data.
 * They are as follows:
 *
 *     max_err = 2.1 * Nops^.50   { Nops in [0, 400] }
 *     max_err = 4.3 * Nops^.435  { Nops in (400, 100000] }
 *
 * Tabulated, here's what this function evaluates to:
 *
 *     MAXIMUM FRAME DRIFT (+/- frames)
 *     ================================
 *      Nops  | dither | no dither
 *     -------+--------+-----------
 *          1 |    2.1 |        1
 *         10 |    6.6 |        5
 *        100 |   21.0 |       50
 *       1000 |   86.8 |      500
 *      10000 |  236.3 |     5000
 *     100000 | 1751.7 |    50000
 *
 * Note that these are WORST CASE.  It is typically much better.
 * (E.g. over 100 ops I usually get 4.5 frames of drift). [2]
 *
 * [1] "Chi-by-eye."
 *
 * [2] As it happens, the fractional part of 'e' is the biggest factor
 *     the determines how much error you will get.  If e is close to a
 *     whole number, you will get very little error by dithering.
 *     When e is midway between two numbers, you are more likely to
 *     get larger errors.
 *
 **********************************************************************
 */

    static inline bool check_frame_drift(double TrueVal,
					 uint32_t Frame,
					 size_t Nops,
					 int Line)
    {
	double max_drift = 0.0;
	bool rv;

	double ActDrift = TrueVal - double(Frame);
	if( Nops <= 1 ) {
	    max_drift = 1.0;
	} else if( Nops <= 400 ) {
	    max_drift = 2.1 * sqrt(double(Nops));
	} else {
	    max_drift = 4.3 * pow(double(Nops), .435);
	}

	rv = (fabs(ActDrift) <= max_drift);

	if( ! rv ) {
	    BOOST_MESSAGE("In " << __FILE__ << "(" << Line << ") "
			  << "Too much drift: True(" << TrueVal << ") "
			  << "- Frame(" << Frame << ") = " << ActDrift
			  << " [Limit is +/- " << max_drift
			  << " for " << Nops << " ops]");
	}

	return rv;
    }

#define DRIFT(t, f, n) check_frame_drift(t, f, n, __LINE__)

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

BOOST_AUTO_TEST_CASE( THIS(010_copy_constructor) )
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

BOOST_AUTO_TEST_CASE( THIS(020_frames_per_tick) )
{
    TX( p.frames_per_tick() == 125.0 );

    p.frame_rate = 123456;
    p.ticks_per_beat = 48;
    p.beats_per_minute = 33.12;
    TX( round(p.frames_per_tick()) == 4659.0 );

    TX( round(x.frames_per_tick()*100.0) == 21882.0 );
}

BOOST_AUTO_TEST_CASE( THIS(030_tick_in_bar) )
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

BOOST_AUTO_TEST_CASE( THIS(040_normalize) )
{
    TransportPosition a;
    double frame;

    // NOTE: normalize() only changes the frame position if it adjusts
    // bbt_offset.

    a = p;
    a.tick += 192 * 6 + 101;
    a.normalize();
    TX( 2 == a.bar );
    TX( 3 == a.beat );
    TX( 101 == a.tick );
    TX( 192 * 4 == a.bar_start_tick );
    TX( DRIFT(0, a.frame, 1) );

    a.beat -= 4;
    a.normalize();
    TX( 1 == a.bar );
    TX( 3 == a.beat );
    TX( 101 == a.tick );
    TX( 0 == a.bar_start_tick );
    TX( DRIFT(0, a.frame, 2) );

    a.bar += 5;
    a.normalize();
    TX( 6 == a.bar );
    TX( 3 == a.beat );
    TX( 101 == a.tick );
    TX( 0 == a.bar_start_tick ); // If you manually change bar, you also have
                                 // to manually adjust bar_start_tick
    TX( DRIFT(0, a.frame, 3) );

    a = p;
    frame = a.frame;
    a.bbt_offset = 6 * 192 * 125 + 50;
    frame += a.bbt_offset - 50;
    a.normalize();
    TX( 2 == a.bar );
    TX( 3 == a.beat );
    TX( 0 == a.tick );
    TX( DRIFT(50.0, a.bbt_offset, 1) );
    TX( 192 * 4 == a.bar_start_tick );
    TX( DRIFT(frame, a.frame, 1) );

    a = x;
    frame = a.frame;
    a.tick -= 99 * 12 + 65;
    a.normalize();
    TX( 347 == a.bar );
    TX( 6 == a.beat );
    TX( 52 == a.tick );
    TX( 115 == a.bbt_offset );
    TX( 0 == a.bar_start_tick );
    TX( DRIFT(frame, a.frame, 1) );

    a = x;
    frame = a.frame;
    a.beat += 13;
    a.normalize();
    TX( 351 == a.bar );
    TX( 4 == a.beat );
    TX( 18 == a.tick );
    TX( 115 == a.bbt_offset );
    TX( 349 + 2 * 7 * 99 == a.bar_start_tick );
    TX( DRIFT(frame, a.frame, 1) );

    a = x;
    a.bar_start_tick = 70000;
    a.bar -= 11;
    a.normalize();  // Does not change bar start tick
    TX( 338 == a.bar );
    TX( 5 == a.beat );
    TX( 18 == a.tick );
    TX( 115 == a.bbt_offset );
    TX( 70000 == a.bar_start_tick );

    a = x;                // 349:5.18.115 (70000)  349.59816096502567
    a.bar_start_tick = 70000;
    frame = a.frame;
    a.bar += 3;           // 352:5.18.115 (70000)  352.59816096502567
    a.beat -= 29;         // 348:4.18.115 (67228)  348.45530382216856
    a.tick += 999;        // 349:7.27.115 (67921)  349.89686226372697
    a.bbt_offset += 3114; // 349:7.41.165 (67921)  349.91739754005869
    frame += 3114.0 - 165.0 + 115.0;
    a.normalize();
    TX( 349 == a.bar );
    TX( 7 == a.beat );
    TX( 41 == a.tick );
    TX( 165 == a.bbt_offset );
    TX( 67921 == a.bar_start_tick );
    TX( DRIFT(frame, a.frame, 1) );    
}

BOOST_AUTO_TEST_CASE( THIS(050_increment) )
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
	TX( DRIFT(frame, p.frame, 1) );
    }

    TX( p.tick == 191 );
    ++p;
    frame += frames_per_tick;
    TX( 1 == p.bar );
    TX( 2 == p.beat );
    TX( 0 == p.tick );
    TX( DRIFT(frame, p.frame, 2) );
    ++p;
    frame += frames_per_tick;
    TX( 1 == p.bar );
    TX( 2 == p.beat );
    TX( 1 == p.tick );
    TX( DRIFT(frame, p.frame, 3) );
    p.bar = 99;
    p.beat = 3;
    ++p;
    frame += frames_per_tick;
    TX( 99 == p.bar );
    TX( 3 == p.beat );
    TX( 2 == p.tick );
    TX( DRIFT(frame, p.frame, 4) );

    // Tests with 'x'
    frame = x.frame;
    frames_per_tick = x.frames_per_tick();
    for( k=x.tick+1 ; (unsigned)k < x.ticks_per_beat ; ++k ) {
	++x;
	frame += frames_per_tick;
	TX( 349 == x.bar );
	TX( 5 == x.beat );
	TX( k == x.tick );
	TX( 115 == x.bbt_offset );
	TX( DRIFT( frame, x.frame, k ) );
    }
    TX( x.tick == 98 );
    ++x;
    frame += frames_per_tick;
    TX( 349 == x.bar );
    TX( 6 == x.beat );
    TX( 0 == x.tick );
    TX( 115 == x.bbt_offset );
    TX( DRIFT( frame, x.frame, k+1 ) );

    x.beat = 7;
    x.tick = 98;
    ++x;
    frame += frames_per_tick;
    TX( 350 == x.bar );
    TX( 1 == x.beat );
    TX( 0 == x.tick );
    TX( 115 == x.bbt_offset );
    TX( DRIFT( frame, x.frame, k+2 ) );
    BOOST_MESSAGE( "++ drift = " << (frame - x.frame) );
}

BOOST_AUTO_TEST_CASE( THIS(060_decrement) )
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
    TX( DRIFT(frame, p.frame, 2) );
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

BOOST_AUTO_TEST_CASE( THIS(070_floor) )
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
    TX( 8 == p.tick );
    TX( DRIFT(954.0, p.frame, 1) );
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
    TX( DRIFT(lastframe, tmp.frame, 1) );
    // Repeating when already floored should
    // give the same result.
    tmp.floor(TransportPosition::TICK);
    TX( tmp.bar == 349 );
    TX( tmp.beat == 5 );
    TX( tmp.tick == 18 );
    TX( tmp.bbt_offset == 0 );
    TX( DRIFT(lastframe, tmp.frame, 1) );

    tmp = x;
    tmp.floor(TransportPosition::BEAT);
    TX( tmp.bar == 349 );
    TX( tmp.beat == 5 );
    TX( tmp.tick == 0 );
    TX( tmp.bbt_offset == 0 );
    lastframe -= 18.0 * tmp.frames_per_tick();
    TX( DRIFT(lastframe, tmp.frame, 1) );
    // Repeating when already floored should
    // give the same result.
    tmp.floor(TransportPosition::BEAT);
    TX( tmp.bar == 349 );
    TX( tmp.beat == 5 );
    TX( tmp.tick == 0 );
    TX( tmp.bbt_offset == 0 );
    TX( DRIFT(lastframe, tmp.frame, 1) );

    tmp = x;
    tmp.floor(TransportPosition::BAR);
    TX( tmp.bar == 349 );
    TX( tmp.beat == 1 );
    TX( tmp.tick == 0 );
    TX( tmp.bbt_offset == 0 );
    lastframe = x.frame - 4.0 * 99.0 * tmp.frames_per_tick()
	- x.tick * tmp.frames_per_tick() - x.bbt_offset;
    TX( DRIFT(lastframe, tmp.frame, 1) );
    // Repeating when already floored should
    // give the same result.
    tmp.floor(TransportPosition::BAR);
    TX( tmp.bar == 349 );
    TX( tmp.beat == 1 );
    TX( tmp.tick == 0 );
    TX( tmp.bbt_offset == 0 );
    TX( DRIFT(lastframe, tmp.frame, 1) );

}

BOOST_AUTO_TEST_CASE( THIS(080_ceil) )
{
    p.ceil(TransportPosition::TICK);
    TX( 1 == p.bar );
    TX( 1 == p.beat );
    TX( 0 == p.tick );
    TX( 0 == p.frame );
    p.ceil(TransportPosition::BEAT);
    TX( 1 == p.bar );
    TX( 1 == p.beat );
    TX( 0 == p.tick );
    TX( 0 == p.frame );
    p.ceil(TransportPosition::BAR);
    TX( 1 == p.bar );
    TX( 1 == p.beat );
    TX( 0 == p.tick );
    TX( 0 == p.frame );

    double fpt = p.frames_per_tick();
    double frame;
    p.tick = 1;
    p.bbt_offset = 75;
    p.frame = round(fpt);
    frame = p.frame + fpt - p.bbt_offset;
    p.ceil(TransportPosition::TICK);
    TX( 1 == p.bar );
    TX( 1 == p.beat );
    TX( 2 == p.tick );
    TX( DRIFT(frame, p.frame, 1) );
    TX( 0 == p.bbt_offset );
    p.tick = 1;
    p.bbt_offset = 921;
    p.frame = round(fpt);
    p.ceil(TransportPosition::BEAT);
    TX( 1 == p.bar );
    TX( 2 == p.beat );
    TX( 0 == p.tick );
    TX( DRIFT(23953.0, p.frame, 2) ); // 2 internal ops.
    TX( 0 == p.bbt_offset );
    p.tick = 1;
    p.bbt_offset = 921;
    p.frame = round(fpt);
    p.ceil(TransportPosition::BAR);
    TX( 2 == p.bar );
    TX( 1 == p.beat );
    TX( 0 == p.tick );
    TX( DRIFT(71955.0, p.frame, 2) ); // 2 internal ops
    TX( 0 == p.bbt_offset );
    TX( 192 * 4 == p.bar_start_tick );

    p.tick = 1;
    p.bbt_offset = 921;
    p.frame = round(fpt/2.0);
    p.ceil(TransportPosition::BAR);
    TX( 3 == p.bar );
    TX( 1 == p.beat );
    TX( 0 == p.tick );
    TX( DRIFT(95891.0, p.frame, 2) );
    TX( 0 == p.bbt_offset );
    TX( 192 * 4 * 2 == p.bar_start_tick );

    // TODO:  ADD MORE TESTS FOR BAR START TICK

    TransportPosition tmp = x;
    double lastframe = tmp.frame;
    tmp.ceil(TransportPosition::TICK);
    TX( tmp.bar == 349 );
    TX( tmp.beat == 5 );
    TX( tmp.tick == 19 );
    TX( tmp.bbt_offset == 0 );
    lastframe += (x.frames_per_tick() - 115.0);
    TX( DRIFT(lastframe, tmp.frame, 1) );
    // Repeating when already ceiled should
    // give the same result.
    tmp.ceil(TransportPosition::TICK);
    TX( tmp.bar == 349 );
    TX( tmp.beat == 5 );
    TX( tmp.tick == 19 );
    TX( tmp.bbt_offset == 0 );
    TX( DRIFT(lastframe, tmp.frame, 1) );

    tmp = x;
    lastframe = tmp.frame;
    tmp.ceil(TransportPosition::BEAT);
    TX( tmp.bar == 349 );
    TX( tmp.beat == 6 );
    TX( tmp.tick == 0 );
    TX( tmp.bbt_offset == 0 );
    lastframe += (99.0 - 18.0) * tmp.frames_per_tick() - 115.0;
    TX( DRIFT(lastframe, tmp.frame, 1) );
    // Repeating when already ceiled should
    // give the same result.
    tmp.ceil(TransportPosition::BEAT);
    TX( tmp.bar == 349 );
    TX( tmp.beat == 6 );
    TX( tmp.tick == 0 );
    TX( tmp.bbt_offset == 0 );
    TX( DRIFT(lastframe, tmp.frame, 1) );

    tmp = x;
    lastframe = tmp.frame;
    lastframe += ((2.0 * 99.0) + (99.0 - 18.0)) * tmp.frames_per_tick()
	- tmp.bbt_offset;
    tmp.ceil(TransportPosition::BAR);
    TX( tmp.bar == 350 );
    TX( tmp.beat == 1 );
    TX( tmp.tick == 0 );
    TX( tmp.bbt_offset == 0 );
    TX( DRIFT(lastframe, tmp.frame, 1) );
    // Repeating when already ceiled should
    // give the same result.
    tmp.ceil(TransportPosition::BAR);
    TX( tmp.bar == 350 );
    TX( tmp.beat == 1 );
    TX( tmp.tick == 0 );
    TX( tmp.bbt_offset == 0 );
    TX( DRIFT(lastframe, tmp.frame, 1) );
}

BOOST_AUTO_TEST_CASE( THIS(090_round) )
{
    TransportPosition a;
    double frame;

    a = p;
    a.round(TransportPosition::TICK);
    TX( 0 == a.frame );
    TX( 1 == a.bar );
    TX( 1 == a.beat );
    TX( 0 == a.tick );
    TX( 0 == a.bbt_offset );
    a.round(TransportPosition::BEAT);
    TX( 0 == a.frame );
    TX( 1 == a.bar );
    TX( 1 == a.beat );
    TX( 0 == a.tick );
    TX( 0 == a.bbt_offset );
    a.round(TransportPosition::BAR);
    TX( 0 == a.frame );
    TX( 1 == a.bar );
    TX( 1 == a.beat );
    TX( 0 == a.tick );
    TX( 0 == a.bbt_offset );

    a = p;
    a.bbt_offset = a.frames_per_tick() / 4.0;
    a.round(TransportPosition::TICK);
    TX( 0 == a.frame );
    TX( 1 == a.bar );
    TX( 1 == a.beat );
    TX( 0 == a.tick );
    TX( 0 == a.bbt_offset );
    a.bbt_offset = a.frames_per_tick() * 3.0 / 4.0;
    frame = a.frame + round(a.frames_per_tick() / 4.0);
    a.round(TransportPosition::TICK);
    TX( DRIFT( frame, a.frame, 1 ) );
    TX( 1 == a.bar );
    TX( 1 == a.beat );
    TX( 1 == a.tick );
    TX( 0 == a.bbt_offset );
    // Re-rounding should not change anything
    a.round(TransportPosition::TICK);
    TX( DRIFT( frame, a.frame, 1 ) );
    TX( 1 == a.bar );
    TX( 1 == a.beat );
    TX( 1 == a.tick );
    TX( 0 == a.bbt_offset );

    // Could use more checks near the zero-point,
    // but we'll move on.

    a = x;
    frame = a.frame;
    frame += a.frames_per_tick() - a.bbt_offset;
    a.round(TransportPosition::TICK);
    TX( 349 == a.bar );
    TX( 5 == a.beat );
    TX( 19 == a.tick );
    TX( 0 == a.bbt_offset );
    TX( DRIFT( frame, a.frame, 1 ) );

    frame -= a.frames_per_tick() * double(a.tick);
    a.round(TransportPosition::BEAT);
    TX( 349 == a.bar );
    TX( 5 == a.beat );
    TX( 0 == a.tick );
    TX( 0 == a.bbt_offset );
    TX( DRIFT( frame, a.frame, 2 ) );

    frame += a.frames_per_tick() * 99.0 * 3.0;
    a.round(TransportPosition::BAR);
    TX( 350 == a.bar );
    TX( 1 == a.beat );
    TX( 0 == a.tick );
    TX( 0 == a.bbt_offset );
    TX( DRIFT( frame, a.frame, 3 ) );

    // Could use more checks... but we'll move on.

}

BOOST_AUTO_TEST_CASE( THIS(100_operator_plus) )
{
    unsigned k;
    TransportPosition a;
    double fpt;
    double frame;

    a = p + (-51);
    TX( 1 == a.bar );
    TX( 1 == a.beat );
    TX( 0 == a.tick );
    TX( 0 == a.bbt_offset );
    TX( 0 == a.frame );

    a = p;
    fpt = a.frames_per_tick();
    for( k=0 ; k<191 ; ++k ) {
	a = a + 1;
	TX( 1 == a.bar );
	TX( 1 == a.beat );
	TX( (k+1) == unsigned(a.tick) );
	TX( DRIFT( double(k+1) * fpt, a.frame, k ) );
    }
    TX( 191 == a.tick );
    ++k;
    a = a + 1;
    TX( 1 == a.bar );
    TX( 2 == a.beat );
    TX( 0 == a.tick );
    TX( DRIFT( double(k) * fpt, a.frame, k ) );

    k = 192 * 2 + 30;
    a = p + k;
    TX( 1 == a.bar );
    TX( 3 == a.beat );
    TX( 30 == a.tick );
    TX( DRIFT( double(k) * fpt, a.frame, 1 ) );


    k = 99 * 5 + 64;
    a = x + k;
    frame = x.frame + (99*5 + 64) * x.frames_per_tick();
    TX( 350 == a.bar );
    TX( 3 == a.beat );
    TX( 82 == a.tick );
    TX( 115 == a.bbt_offset );
    TX( DRIFT( frame, a.frame, 1 ) );

    a = a + (-k);
    frame = x.frame;
    TX( 349 == a.bar );
    TX( 5 == a.beat );
    TX( 18 == a.tick );
    TX( 115 == a.bbt_offset );
    TX( DRIFT( frame, a.frame, 2 ) );
}

BOOST_AUTO_TEST_CASE( THIS(110_operator_minus) )
{
    unsigned k;
    TransportPosition a;
    double fpt;

    a = p - 51;
    TX( 1 == a.bar );
    TX( 1 == a.beat );
    TX( 0 == a.tick );
    TX( 0 == a.bbt_offset );
    TX( 0 == a.frame );

    a = p;
    a.bar = 2;
    a.beat = 1;
    a.tick = 0;
    a.frame = 2000000;
    fpt = a.frames_per_tick();
    for( k=192 ; k>0 ; --k ) {
	a = a - 1;
	TX( 1 == a.bar );
	TX( 4 == a.beat );
	TX( (k-1) == unsigned(a.tick) );
	TX( DRIFT( 2000000.0 - (double(192-k+1) * fpt), a.frame, (193-k) ) );
    }
    TX( 0 == a.tick );
    a = a - 1;
    k = 193;
    TX( 1 == a.bar );
    TX( 3 == a.beat );
    TX( 191 == a.tick );
    TX( DRIFT( 2000000.0 - (double(k) * fpt), a.frame, k ) );

    k = 99 * 2 + 30;
    a = x - k;
    fpt = x.frames_per_tick();
    TX( 349 == a.bar );
    TX( 2 == a.beat );
    TX( 87 == a.tick );
    TX( 115 == a.bbt_offset );
    TX( DRIFT( double(x.frame) - (double(k) * fpt), a.frame, 1 ) );

    k = 99 * 5 + 64;
    a = x - k;
    TX( 348 == a.bar );
    TX( 6 == a.beat );
    TX( 53 == a.tick );
    TX( 115 == a.bbt_offset );
    TX( DRIFT( double(x.frame) - double(k) * fpt, a.frame, 1 ) );

    a = a - (-k);
    TX( 349 == a.bar );
    TX( 5 == a.beat );
    TX( 18 == a.tick );
    TX( 115 == a.bbt_offset );
    TX( DRIFT( double(x.frame), a.frame, 2 ) );
}

BOOST_AUTO_TEST_CASE( THIS(120_operator_plus_equals) )
{
    unsigned k;
    TransportPosition a;
    double fpt;

    a = p;
    fpt = a.frames_per_tick();
    for( k=0 ; k<191 ; ++k ) {
	a += 1;
	TX( 1 == a.bar );
	TX( 1 == a.beat );
	TX( k+1 == unsigned(a.tick) );
	TX( DRIFT( double(k+1) * fpt, a.frame, k ) );
    }
    TX( 191 == a.tick );
    ++k;
    a += 1;
    TX( 1 == a.bar );
    TX( 2 == a.beat );
    TX( 0 == a.tick );
    TX( DRIFT( double(k) * fpt, a.frame, k ) );

    k = 192 * 2 + 30;
    a = p;
    a += k;
    TX( 1 == a.bar );
    TX( 3 == a.beat );
    TX( 30 == a.tick );
    TX( DRIFT( double(k) * fpt, a.frame, 1 ) );

    k = 99 * 5 + 64;
    a = x;
    a += k;
    fpt = a.frames_per_tick();
    TX( 350 == a.bar );
    TX( 3 == a.beat );
    TX( 82 == a.tick );
    TX( 115 == a.bbt_offset );
    TX( DRIFT( double(x.frame) + double(k) * fpt, a.frame, 1 ) );

    a += (-k);
    TX( 349 == a.bar );
    TX( 5 == a.beat );
    TX( 18 == a.tick );
    TX( 115 == a.bbt_offset );
    TX( DRIFT( double(x.frame), a.frame, 2 ) );
}

BOOST_AUTO_TEST_CASE( THIS(130_operator_minus_equals) )
{
    unsigned k;
    TransportPosition a;
    double fpt;

    a = p;
    a -= 51;
    TX( 1 == a.bar );
    TX( 1 == a.beat );
    TX( 0 == a.tick );
    TX( 0 == a.bbt_offset );
    TX( 0 == a.frame );

    a = p;
    a.bar = 2;
    a.beat = 1;
    a.tick = 0;
    a.frame = 2000000;
    fpt = a.frames_per_tick();
    for( k=192 ; k>0 ; --k ) {
	a -= 1;
	TX( 1 == a.bar );
	TX( 4 == a.beat );
	TX( (k-1) == unsigned(a.tick) );
	TX( DRIFT( 2000000.0 - (double(192-k+1) * fpt), a.frame, (193-k) ) );
    }
    TX( 0 == a.tick );
    a -= 1;
    k = 193;
    TX( 1 == a.bar );
    TX( 3 == a.beat );
    TX( 191 == a.tick );
    TX( DRIFT( 2000000.0 - (double(k) * fpt), a.frame, k ) );

    k = 99 * 2 + 30;
    a = x;
    a -= k;
    fpt = x.frames_per_tick();
    TX( 349 == a.bar );
    TX( 2 == a.beat );
    TX( 87 == a.tick );
    TX( 115 == a.bbt_offset );
    TX( DRIFT( double(x.frame) - (double(k) * fpt), a.frame, 1 ) );

    k = 99 * 5 + 64;
    a = x;
    a -= k;
    TX( 348 == a.bar );
    TX( 6 == a.beat );
    TX( 53 == a.tick );
    TX( 115 == a.bbt_offset );
    TX( DRIFT( double(x.frame) - double(k) * fpt, a.frame, 1 ) );

    a -= (-k);
    TX( 349 == a.bar );
    TX( 5 == a.beat );
    TX( 18 == a.tick );
    TX( 115 == a.bbt_offset );
    TX( DRIFT( double(x.frame), a.frame, 2 ) );
}

BOOST_AUTO_TEST_SUITE_END()
