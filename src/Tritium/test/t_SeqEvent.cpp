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

#include <Tritium/SeqEvent.hpp>
#include <Tritium/Instrument.hpp>
#include <Tritium/Logger.hpp>
#include <cmath>
#include <memory>

#define THIS_NAMESPACE t_SeqEvent
#include "test_macros.hpp"

using namespace Tritium;

namespace THIS_NAMESPACE
{

    struct Fixture
    {
	SeqEvent ev;  // This is the "normal" one.
	SeqEvent xev; // This is the odd one.
	std::auto_ptr<Instrument> instr;  // Note() won't let us set a null instrument

	Fixture() : ev(), xev() {
	    Logger::create_instance();
	    instr.reset( Instrument::create_empty() );

	    ev.note.set_instrument( instr.get() );

	    xev.frame = 0xFEEEEEEE;
	    xev.type = SeqEvent::ALL_OFF;
	    xev.quantize = true;
	    xev.instrument_index = 0xFF;
	    xev.note.set_instrument( instr.get() );
	}

	~Fixture() {
	}
    };

} // namespace THIS_NAMESPACE

TEST_BEGIN( Fixture );

TEST_CASE( 001_defaults )
{
    // Test the defaults
    CK( ev.frame == 0 );
    CK( ev.type == SeqEvent::NOTE_ON );
    CK( ev.quantize == false );
    CK( ev.instrument_index == 0 );
}

TEST_CASE( 002_copy )
{
    SeqEvent cp;
    cp = ev;
    CK( cp.frame == ev.frame );
    CK( cp.type == ev.type );
    // CK( cp.note == ev.note );  // TODO
    CK( cp.quantize == ev.quantize );
    CK( cp.instrument_index == ev.instrument_index );

    cp = xev;
    CK( cp.frame == xev.frame );
    CK( cp.type == xev.type );
    // CK( cp.note == xev.note ); // TODO
    CK( cp.quantize == xev.quantize );
    CK( cp.instrument_index == xev.instrument_index );

    // Verify independence
    ++cp.frame;
    cp.type = SeqEvent::NOTE_OFF;
    cp.quantize = !cp.quantize;
    ++cp.instrument_index;
    CK( cp.frame != xev.frame );
    CK( cp.type != xev.type );
    // CK( cp.note != xev.note ); // TODO
    CK( cp.quantize != xev.quantize );
    CK( cp.instrument_index != xev.instrument_index );

    xev = ev;
    CK( ev.frame == xev.frame );
    CK( ev.type == xev.type );
    // CK( ev.note == xev.note ); // TODO
    CK( ev.quantize == xev.quantize );
    CK( ev.instrument_index == xev.instrument_index );

    // Confirm xev
    CK( xev.frame == 0 );
    CK( xev.type == SeqEvent::NOTE_ON );
    CK( xev.quantize == false );
    CK( xev.instrument_index == 0 );
}

TEST_CASE( 003_less )
{
    CK( less(ev, xev) );    
    CK( ! less(xev, ev) );
    xev.frame = ev.frame;
    CK( ! less(ev, xev) );
    CK( ! less(ev, xev) );
    ev.frame = xev.frame+1;
    CK( ! less(ev, xev) );
    CK( less(xev, ev) );
}

TEST_CASE( 004_compare )
{
    SeqEvent a;
    SeqEvent b = xev;

    a.note.set_instrument( instr.get() );

    CK( a == ev );
    CK( ev == a );
    CK( ! (a != ev) );
    CK( ! (ev != a) );

    CK( b == xev );
    CK( xev == b );
    CK( ! (b != xev) );
    CK( ! (xev != b) );

    CK( ev != xev );
    CK( xev != ev );
    CK( ! (ev == xev) );
    CK( ! (xev == ev) );

    a.frame = 1234;
    CK( a != ev );
    CK( ev != a );
    CK( ! (a == ev) );
    CK( ! (ev == a) );
}

TEST_END()
