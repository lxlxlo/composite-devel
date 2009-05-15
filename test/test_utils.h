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
#ifndef __HYDROGEN_TEST_UTILS__
#define __HYDROGEN_TEST_UTILS__

#include <stdint.h>
#include <cstring>
#include <boost/test/unit_test.hpp>

/*
 * test_utils.h
 *
 * When writing tests, often a macro or function will be useful
 * for testing states of objects.  Instead of copying the code
 * into each test, they are gathered here so that they can
 * be reused and updated uniformly.
 */

namespace H2Test
{

    /*
     * check_frame_drift()
     *
     * Whan calculations with TransportPosition manipulate the frame,
     * a little drift is inherent.  This function checks to see if it
     * is within expected bounds.
     *
     * It is recommended that you use the H2TEST_DRIFT() macro
     * instead of this function.
     *
     * Parameters:  TrueVal - The expected frame value.
     *              Frame - The actual frame value (e.g. TransportPosition::frame)
     *              Nops - The number of frame manipulation operations that
     *                     have occurred up to now.
     *              File - A source code file name reference.
     *              Line - A source code line number reference.
     *
     * Returns: true if it is within expected tolerances.  false if not.
     *
     * See test_utils.cpp for documentation.
     *
     */
    bool check_frame_drift(double TrueVal,
			   uint32_t Frame,
			   size_t Nops,
			   const char* File,
			   int Line);

} // namespace H2Test

#define H2TEST_DRIFT(t, f, n) H2Test::check_frame_drift(t, f, n, __FILE__, __LINE__)

#define H2TEST_VALID_POS(p, s) {					\
    BOOST_REQUIRE( typeid(p) == typeid(H2Core::TransportPosition) );	\
    BOOST_REQUIRE( typeid(s) == typeid(H2Core::Song*) );		\
    CK( (p).bar > 0 );							\
    CK( (p).bar <= H2Core::song_bar_count(s) );				\
    CK( (p).ticks_per_beat == 48 );					\
    CK( (p).beats_per_bar == (H2Core::ticks_in_bar((s),(p).bar)/48) );	\
    CK( (p).beat_type == 4 );						\
    CK( (p).beat > 0 );							\
    CK( (p).beat <= (p).beats_per_bar );				\
    CK( (p).tick < ticks_in_bar((s), (p).bar) );			\
    CK( (p).bbt_offset < (p).frames_per_tick() );			\
    CK( (p).bar_start_tick < H2Core::song_tick_count(s) );		\
    CK( (p).bar_start_tick == H2Core::bar_start_tick((s), (p).bar) );	\
    CK( ((p).bar_start_tick + (p).tick) < H2Core::song_tick_count(s) );	\
}


#endif // __HYDROGEN_TEST_UTILS__
