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

#include <hydrogen/SeqEvent.h>
#include <boost/test/unit_test.hpp>

#include <cmath>

using namespace H2Core;

// "BOOST_CHECK( foo )" is too much typing....
#define TX BOOST_CHECK

struct t_SeqEvent_Fixture
{
    SeqEvent ev;  // This is the "normal" one.
    SeqEvent xev; // This is the odd one.

    t_SeqEvent_Fixture() : ev(), xev() {

	xev.frame = 0xFEEEEEEE;
	xev.type = SeqEvent::ALL_OFF;
	xev.quantize = true;
	xev.instrument_index = 0xFF;
    }

    ~t_SeqEvent_Fixture() {}
};

BOOST_FIXTURE_TEST_SUITE( t_SeqEvent, t_SeqEvent_Fixture );

BOOST_AUTO_TEST_CASE( t_001_defaults )
{
    // Test the defaults
    TX( ev.frame == 0 );
    TX( ev.type == SeqEvent::NOTE_ON );
    TX( ev.quantize == false );
    TX( ev.instrument_index == 0 );
}

BOOST_AUTO_TEST_CASE( t_002_copy )
{
    SeqEvent cp;
    cp = ev;
    TX( cp.frame == ev.frame );
    TX( cp.type == ev.type );
    // TX( cp.note == ev.note );  // TODO
    TX( cp.quantize == ev.quantize );
    TX( cp.instrument_index == ev.instrument_index );

    cp = xev;
    TX( cp.frame == xev.frame );
    TX( cp.type == xev.type );
    // TX( cp.note == xev.note ); // TODO
    TX( cp.quantize == xev.quantize );
    TX( cp.instrument_index == xev.instrument_index );

    // Verify independence
    ++cp.frame;
    cp.type = SeqEvent::NOTE_OFF;
    cp.quantize = !cp.quantize;
    ++cp.instrument_index;
    TX( cp.frame != xev.frame );
    TX( cp.type != xev.type );
    // TX( cp.note != xev.note ); // TODO
    TX( cp.quantize != xev.quantize );
    TX( cp.instrument_index != xev.instrument_index );

    xev = ev;
    TX( ev.frame == xev.frame );
    TX( ev.type == xev.type );
    // TX( ev.note == xev.note ); // TODO
    TX( ev.quantize == xev.quantize );
    TX( ev.instrument_index == xev.instrument_index );

    // Confirm xev
    TX( xev.frame == 0 );
    TX( xev.type == SeqEvent::NOTE_ON );
    TX( xev.quantize == false );
    TX( xev.instrument_index == 0 );
}

BOOST_AUTO_TEST_CASE( t_003_less )
{
    TX( less(ev, xev) );    
    TX( ! less(ev, xev) );
    xev.frame = ev.frame;
    TX( ! less(ev, xev) );
    TX( ! less(ev, xev) );
    ev.frame = xev.frame+1;
    TX( ! less(ev, xev) );
    TX( less(ev, xev) );
}

BOOST_AUTO_TEST_SUITE_END()
