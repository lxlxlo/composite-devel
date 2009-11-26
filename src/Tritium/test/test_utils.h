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
#ifndef TRITIUMTEST_TEST_UTILS_H
#define TRITIUMTEST_TEST_UTILS_H

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

namespace Tritium {
    struct TransportPosition;
    class Song;
}

namespace TritiumTest
{
    /*
     * check_frame_drift()
     *
     * Whan calculations with TransportPosition manipulate the frame,
     * a little drift is inherent.  This function checks to see if it
     * is within expected bounds.
     *
     * It is recommended that you use the TT_DRIFT() macro
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

    /*
     * valid_position()
     *
     * Tests that the B:b.t info in the struct 'p' describes some
     * point within the song 's'.  E.g. if the song is only 8 bars
     * long, but p->bar is 12... then p is not within the song and
     * this function will return false.  However, if p->bar were 7,
     * it would be OK.
     *
     * Another example, if bar 5 of the song has only 2 beats, but
     * p->bar == 5 and p->beat == 3, then this function will return
     * false.
     *
     * Parameters: p - A TransportPosition struct (e.g. one filled out
     *                 by Transport::get_position();
     *             s - A Song object.
     *
     * Returns:  true if p is within the bounds of s.  false if the
     *           position p is outside the song.
     */
    bool valid_position(Tritium::TransportPosition& p, Tritium::Song* s);

} // namespace TritiumTest

// Convenience macro for check_frame_drift()
#define TT_DRIFT(t, f, n) TritiumTest::check_frame_drift(t, f, n, __FILE__, __LINE__)

// This tests that the positions is valid for the given song.
// Transports should always give a BBT within the current song.
//
// This is wrapped inside a BOOST_CHECK so that the user gets
// feedback on the line number where it happened.
#define TT_VALID_POS(p, s) BOOST_CHECK(TritiumTest::valid_position(p, s))

#endif // TRITIUMTEST_TEST_UTILS_H
