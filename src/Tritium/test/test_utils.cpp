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

#include "test_utils.hpp"
#include <Tritium/TransportPosition.hpp>
#include <Tritium/Song.hpp>
#include <cmath>

using namespace Tritium;

namespace TritiumTest
{

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

    bool check_frame_drift(double TrueVal,
			   uint32_t Frame,
			   size_t Nops,
			   const char* File,
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
	    BOOST_MESSAGE("In " << File << "(" << Line << ") "
			  << "Too much drift: True(" << TrueVal << ") "
			  << "- Frame(" << Frame << ") = " << ActDrift
			  << " [Limit is +/- " << max_drift
			  << " for " << Nops << " ops]");
	}

	return rv;
    }

    #define XCK(expr) { BOOST_CHECK( expr ); rv = rv && (expr); }

    bool valid_position(TransportPosition& p, Song* s)
    {
	bool rv = true;
	XCK( p.bar > 0 );
	XCK( p.bar <= s->song_bar_count() );
	XCK( p.ticks_per_beat == 48 );
	XCK( p.beats_per_bar == (s->ticks_in_bar(p.bar)/48) );
	XCK( p.beat_type == 4 );
	XCK( p.beat > 0 );
	XCK( p.beat <= p.beats_per_bar );
	XCK( p.tick < s->ticks_in_bar(p.bar) );
	XCK( p.bbt_offset < p.frames_per_tick() );
	XCK( p.bar_start_tick < s->song_tick_count() );
	XCK( p.bar_start_tick == s->bar_start_tick(p.bar) );
	XCK( (p.bar_start_tick + p.tick) < s->song_tick_count() );
	return rv;
    }

} // namespace TritiumTest
