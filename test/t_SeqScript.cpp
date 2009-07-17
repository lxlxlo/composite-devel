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

/**
 * This test will test both the SeqScript and
 * SeqScript iterator classes... because they are
 * integral.
 */

#include <cstdlib>
#include <time.h>
#include <hydrogen/SeqScript.h>
#include <hydrogen/instrument.h>
#include <memory>

// CHANGE THIS TO MATCH YOUR FILE:
#define THIS_NAMESPACE t_SeqScript
#include "test_macros.h"

using namespace H2Core;

namespace THIS_NAMESPACE
{

    struct Fixture
    {
	struct pat_t {
	    uint32_t frame;
	    int on_off;
	    int inst;
	    float vel;
	};

	SeqScript x; // Simple object.
	// Need a more complex object with random insertion.

	std::auto_ptr<Instrument> inst_refs[3];
	const size_t x_pat_size;
	pat_t* x_pat;  // null terminated.

	Fixture() : x_pat_size(8) {
	    Logger::create_instance();
	    /*------------------------------------------
	     * Initialize dummy instruments
	     *------------------------------------------
	     */

	    inst_refs[0].reset( Instrument::create_empty() );
	    inst_refs[1].reset( Instrument::create_empty() );
	    inst_refs[2].reset( Instrument::create_empty() );

	    /*------------------------------------------
	     * Initialize master pattern
	     *------------------------------------------
	     */

	    // Pattern for X.
	    //    . . . . . . . .
	    // 0: X   X   X   X
	    // 1:   X   XX  X   XX
	    // 2:     X       X
	    //
	    // Each . is 24000 frames
	    x_pat = new pat_t[x_pat_size];

	    pat_t t_x_pat[] = {
		{     0, 1, 0, 1.0 },
		{ 24000, 1, 1,  .7 },
		{ 48000, 1, 0, 1.0 },
		{ 48000, 1, 2,  .5 },
		{ 60000, 0, 1, 1.0 }, // Note off
		{ 72000, 1, 2,  .7 },
		{ 84000, 1, 2,  .5 },
		{ 84000, 0, 1, 0.0 }
		/* Repeat @ frame 96000 */
	    };

	    int k;
	    for( k=0 ; k<x_pat_size ; ++k ) {
		x_pat[k] = t_x_pat[k];
	    }

	    // Fill pattern X in an ordered manner.
	    // Repeat x_pat 4 times.
	    x.reserve(1024);
	    for( k=0 ; k < 4*x_pat_size ; ++k ) {
		SeqEvent tmp;
		int p = k % x_pat_size;
		tmp.frame = x_pat[p].frame + 96000 * (k/x_pat_size);
		tmp.type = (x_pat[p].on_off) ? SeqEvent::NOTE_ON : SeqEvent::NOTE_OFF;
		tmp.note.set_instrument( inst_refs[ x_pat[p].inst ].get() );
		tmp.note.set_velocity(x_pat[p].vel);
		tmp.instrument_index = x_pat[p].inst;
		x.insert(tmp);
	    }

	}

	~Fixture() {
	    delete[] x_pat;
	}
    };

} // namespace THIS_NAMESPACE

TEST_BEGIN( Fixture );

TEST_CASE( 010_defaults )
{
    srand(time(0));

    SeqScript a;
    CK( a.empty() );
    CK( a.size() == 0 );
    CK( a.size( rand() ) == 0 );
    CK( a.max_size() == 1024 );
    CK( a.begin_const() == a.end_const() );
    CK( a.begin_const() == a.end_const( rand() ) );

}

TEST_CASE( 020_size_and_storage )
{
    SeqScript a;

    CK( a.empty() );

    a.reserve(1024);
    CK( a.empty() );
    CK( a.size() == 0 );
    CK( a.size( rand() ) == 0 );
    CK( a.max_size() == 1024 );
    CK( a.begin_const() == a.end_const() );
    CK( a.begin_const() == a.end_const( rand() ) );

    a.clear(); // Should have no affect.
    CK( a.empty() );
    CK( a.size() == 0 );
    CK( a.size( rand() ) == 0 );
    CK( a.max_size() == 1024 );
    CK( a.begin_const() == a.end_const() );
    CK( a.begin_const() == a.end_const( rand() ) );

    CK( ! x.empty() );
    CK( x.size() == 32 );
    CK( x.size(1024) == 1 );
    CK( x.size(60000) == 4 );
    CK( x.max_size() == 1024 );
    CK( x.begin_const() != x.end_const() );
    CK( x.begin_const() != x.end_const(1024) );
    CK( x.end_const() != x.end_const(1024) );
    CK( x.begin_const() != x.end_const(48000) );
    CK( x.end_const() == x.end_const(999999) );

    // Resizing (in any way) will delete contents.
    x.reserve( 2048 );
    CK( x.empty() );
    CK( x.size() == 0 );
    CK( x.size(1024) == 0 );
    CK( x.size(60000) == 0 );
    CK( x.max_size() == 2048 );
    CK( x.begin_const() == x.end_const() );
    CK( x.begin_const() == x.end_const(1024) );
    CK( x.begin_const() == x.end_const(48000) );

    x.insert(SeqEvent());  // TODO:  These two lines fail.
    CK( x.size() == 1 );

    x.clear();             // TODO:  This fails on an assertion.
    CK( x.empty() );
    CK( x.size() == 0 );
    CK( x.size(1024) == 0 );
    CK( x.size(60000) == 0 );
    CK( x.max_size() == 2048 );
    CK( x.begin_const() == x.end_const() );
    CK( x.begin_const() == x.end_const(1024) );
    CK( x.begin_const() == x.end_const(48000) );

}

TEST_CASE( 030_check_sorting )
{
    SeqScriptConstIterator cur, prev;
    for( cur = x.begin_const() ; cur != x.end_const() ; ++cur ) {
	if( cur == x.begin_const() ) {
	    prev = cur;
	    continue;
	}
	CK( prev->frame <= cur->frame );
	prev = cur;	
    }
}

TEST_CASE( 040_check_contents )
{
    // Refer to the fixture to understand this.
    int j, k;
    SeqScriptConstIterator cur;
    uint32_t frame;

    CK( x.size() == 4 * x_pat_size );

    cur = x.begin_const();
    for( j=0 ; j < 4 ; ++j ) {
	frame = j * 96000;
	for( k = 0 ; k < x_pat_size ; ++k ) {
	    CK( cur != x.end_const() );
	    if( cur == x.end_const() ) {
		CK( false );
		break;
	    }
	    CK( cur->frame == x_pat[k].frame + frame );
	    CK( cur->type == ((x_pat[k].on_off) ? SeqEvent::NOTE_ON : SeqEvent::NOTE_OFF) );
	    CK( cur->note.get_instrument() == inst_refs[ x_pat[k].inst ].get() );
	    CK( cur->note.get_velocity() == x_pat[k].vel );
	    CK( cur->instrument_index == x_pat[k].inst );
	    ++cur;
	}
    }

}

TEST_CASE( 050_consumed_pass_1 )
{
    // Just check that stuff is getting removed.
    // Next test will check frame manipulation.
    uint32_t frame = 0, delta = 12000;
    int k, count;

    count = x_pat_size * 4;
    CK( x.size() == count );
    for( k = 0 ; k < 4 ; ++k ) {
	x.consumed( delta );
	frame += delta;   // @ 12000
	count -= 1;       // from frame 0
	CK( x.size() == count );
	x.consumed( delta );
	frame += delta;   // @ 24000
	CK( x.size() == count );
	x.consumed( delta );
	frame += delta;   // @ 36000
	count -= 1;       // frome frame 24000
	CK( x.size() == count );
	x.consumed( delta );
	frame += delta;   // @ 48000
	CK( x.size() == count );
	x.consumed( delta );
	frame += delta;   // @ 60000
	count -= 2;       // from frame 48000
	CK( x.size() == count );
	x.consumed( delta );
	frame += delta;   // @ 72000
	count -= 1;        // from frame 60000
	CK( x.size() == count );
	x.consumed( delta );
	frame += delta;   // @ 84000
	count -= 1;       // from frame 72000
	CK( x.size() == count );
	x.consumed( delta );
	frame += delta;   // @ 96000
	count -= 2;       // from frame 84000
	CK( x.size() == count );

	CK( count >= 0 );
    }
    CK( x.empty() );
}

TEST_CASE( 060_consumed_pass_2 )
{
    CK( false );  // TODO:  Need to add tests.
}

// Need more tests.

TEST_END()
