/*
 * Tritium
 * Copyright(c) 2009 by Gabriel M. Beddingfield <gabriel@teuton.org>
 *
 * http://gabe.is-a-geek.org/composite/
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

#include "../src/SeqScriptPrivate.h"
#include <Tritium/instrument.h>
#include <vector>
#include <memory>
#include <map>
#include <algorithm>

// CHANGE THIS TO MATCH YOUR FILE:
#define THIS_NAMESPACE t_SeqScriptPrivate
#include "test_macros.h"

using namespace Tritium;

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

	SeqScriptPrivate x; // Simple object
	SeqScriptPrivate r; // same as 'x' but randomly inserted
	// Need a more complex object with random insertion.

	std::auto_ptr<Instrument> inst_refs[3];
	const size_t x_pat_size;
	pat_t* x_pat;

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
	    x.reserve(512);
	    for( k=0 ; k < 4*x_pat_size ; ++k ) {
		SeqEvent tmp;
		int p = k % x_pat_size;
		tmp.frame = x_pat[p].frame + 96000 * (k/x_pat_size);
		tmp.type = (x_pat[p].on_off) ? SeqEvent::NOTE_ON : SeqEvent::NOTE_OFF;
		tmp.note.set_instrument( inst_refs[ x_pat[p].inst ].get() );
		tmp.note.set_velocity(x_pat[p].vel);
		x.insert(tmp);
	    }

	    /*-----------------------------------------
	     * Create a random vector for the 'r' object.
	     *-----------------------------------------
	     */
	    std::vector<int> r_int;
	    for(k=0 ; k < (4*x_pat_size) ; ++k)
		r_int.push_back(k);

	    std::random_shuffle(r_int.begin(), r_int.end());

	    /*-----------------------------------------
	     * Fill the 'r' object with data.
	     *-----------------------------------------
	     */
	    std::vector<int>::iterator n;
	    for( n = r_int.begin() ; n != r_int.end() ; ++n ) {
		SeqEvent tmp;
		int p = (*n) % x_pat_size;
		tmp.frame = x_pat[p].frame;
		tmp.frame += 96000 * ((*n)/x_pat_size);
		tmp.type = (x_pat[p].on_off) ? SeqEvent::NOTE_ON : SeqEvent::NOTE_OFF;
		tmp.note.set_instrument( inst_refs[ x_pat[p].inst ] .get() );
		tmp.note.set_velocity(x_pat[p].vel);
		r.insert(tmp);
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
    SeqScriptPrivate a;

    CK( a.empty() );
    CK( a.size() == 0 );
    CK( a.max_size() == 1024 );
    CK( a.begin() == a.end() );
    CK( ! (a.begin() != a.end()) );

    CK( ! x.empty() );
    CK( x.size() == x_pat_size * 4 );
    CK( x.max_size() == 512 );
    CK( x.begin() != x.end() );
    CK( ! (x.begin() == x.end()) );

    CK( ! r.empty() );
    CK( r.size() == x_pat_size * 4 );
    CK( r.max_size() == 1024 );
    CK( r.begin() != r.end() );
    CK( ! (r.begin() == r.end()) );
}

TEST_CASE( 020_sort_order )
{
    SeqScriptPrivate::iterator prev, cur;

    cur = prev = x.begin();
    ++cur;
    while( cur != x.end() ) {
	CK( ! ((cur->ev) < (prev->ev)) );
	++cur; ++prev;
    }

    cur = prev = r.begin();
    ++cur;
    while( cur != r.end() ) {
	CK( ! ((cur->ev) < (prev->ev)) );
	++cur; ++prev;
    }
}

TEST_CASE( 030_contents )
{
    int k;
    pat_t *beg = x_pat, *end = x_pat + x_pat_size, *x_cur;
    SeqScriptPrivate::iterator cur = x.begin();
    for( k=0 ; k<4 ; ++k ) {
	for(x_cur = beg; x_cur != end ; ++x_cur) {
	    CK( cur != x.end() );
	    CK( (cur->ev.frame - 96000 * k) == x_cur->frame );
	    ++cur;
	}
    }
    CK( cur == x.end() );

    // For any given frame #, we don't have any strict requirements
    // for how the events are ordered.  Therefore, the following
    // code is a little complicated... so that we can handle
    // this potential for random order in the same frame.
    //
    // To handle it, the 'x' and 'r' objects are converted to
    // multimaps (indexed by frame #).  This helps us divide up the
    // work, making comparisons one frame at a time.

    // To compare x and r, we'll use a multimap
    typedef std::multimap<SeqEvent::frame_type, SeqEvent> mm_t;
    typedef std::pair<SeqEvent::frame_type, SeqEvent> p_t;
    typedef std::pair<SeqEvent::frame_type, SeqEvent> p_t;

    mm_t x_map, r_map;
    for( cur = x.begin() ; cur != x.end() ; ++cur )
	x_map.insert(p_t(cur->ev.frame, cur->ev));
    for( cur = r.begin() ; cur != r.end() ; ++cur )
	r_map.insert(p_t(cur->ev.frame, cur->ev));

    CK( x_map.size() == r_map.size() );

    // Get all the the multimap keys.
    std::map<SeqEvent::frame_type, int> keys;
    mm_t::iterator x_iter, r_iter;
    for( x_iter = x_map.begin() ; x_iter != x_map.end() ; ++x_iter )
	keys[x_iter->first] = 1;
    for( r_iter = r_map.begin() ; r_iter != r_map.end() ; ++r_iter )
	keys[r_iter->first] = 1;

    std::map<SeqEvent::frame_type, int>::iterator key_iter;
    SeqEvent::frame_type frame;
    for( key_iter = keys.begin() ; key_iter != keys.end() ; ++key_iter ) {
	frame = key_iter->first;

	CK( x_map.count( frame ) == r_map.count( frame ) );

	if( x_map.count( frame ) == 1 ) {
	    SeqEvent &x_ev = x_map.find(frame)->second;
	    SeqEvent &r_ev = r_map.find(frame)->second;
	    CK( x_ev.frame == frame );
	    CK( r_ev.frame == frame );
	    CK( x_ev == r_ev );
	} else {
	    // Comparing multi-events-per frame is harder to do,
	    // so I've segregated the code to here.
	    std::vector<SeqEvent> x_evs, r_evs;
	    std::pair<mm_t::iterator, mm_t::iterator> range;
	    range = x_map.equal_range(frame);
	    for( mm_t::iterator tmp = range.first ; tmp != range.second ; ++tmp ) {
		x_evs.push_back( tmp->second );
	    }
	    range = r_map.equal_range(frame);
	    for( mm_t::iterator tmp = range.first ; tmp != range.second ; ++tmp ) {
		r_evs.push_back( tmp->second );
	    }

	    CK( x_evs.size() == x_map.count( frame ) );
	    CK( r_evs.size() == r_map.count( frame ) );
	    std::vector<SeqEvent>::iterator r_pos, x_pos;
	    for( x_pos = x_evs.begin() ; x_pos != x_evs.end() ; ++x_pos ) {
		bool had_hit = false;
		for( r_pos = r_evs.begin() ; r_pos != r_evs.end() ; ++r_pos ) {
		    if( *x_pos == *r_pos ) {
			r_evs.erase( r_pos );
			had_hit = true;
			break;
		    }
		}
		CK( had_hit );
	    }
	    CK( r_evs.size() == 0 );
	}
    }
}

TEST_CASE( 040_frame_adjustment_counts )
{
    // Test that as we remove frames, the event count is resized
    // appropriately.
    //
    // The x_pat has frames that are all multiples of 12000
    // (prime factorization:  2^5 3 5^3)
    //
    // Therefore we'll use a period size of 121 (11^2) to have
    // non-alignment.

    SeqEvent::frame_type frame, delta = 121;
    SeqEvent::frame_type aligned;
    size_t repeat = 0;
    int deleted, left;
    for( frame = 0 ; frame < 96000*5 ; frame += delta ) {
	aligned = frame % 96000;
	repeat = frame / 96000;
	if( aligned == 0 ) {
	    deleted = 0;
	} else if (aligned < 24000) {
	    deleted = 1;
	} else if (aligned < 48000) {
	    deleted = 2;
	} else if (aligned < 60000) {
	    deleted = 4;
	} else if (aligned < 72000) {
	    deleted = 5;
	} else if (aligned < 84000) {
	    deleted = 6;
	} else {
	    deleted = 8;
	}	
	left = 32 - int(repeat * 8) - deleted;
	if( left < 0 ) left = 0;
	CK( x.size() == unsigned(left) );

	x.consumed(delta);
    }

}

TEST_CASE( 050_size_and_storage )
{
    SeqScriptPrivate a;

    CK( a.empty() );

    a.reserve(128);
    CK( a.empty() );
    CK( a.size() == 0 );
    CK( a.max_size() == 128 );
    CK( a.begin() == a.end() );

    a.clear(); // Should have no affect.
    CK( a.empty() );
    CK( a.size() == 0 );
    CK( a.max_size() == 128 );
    CK( a.begin() == a.end() );

    CK( ! x.empty() );
    CK( x.size() == 32 );
    CK( x.max_size() == 512 );
    CK( x.begin() != x.end() );

    // Resizing (in any way) will delete contents.
    x.reserve( 2048 );
    CK( x.empty() );
    CK( x.size() == 0 );
    CK( x.max_size() == 2048 );
    CK( x.begin() == x.end() );

    x.insert(SeqEvent());  // TODO:  These two lines fail.
    CK( x.size() == 1 );

    x.clear();             // TODO:  This fails on an assertion.
    CK( x.empty() );
    CK( x.size() == 0 );
    CK( x.max_size() == 2048 );
    CK( x.begin() == x.end() );
    
}

TEST_END()
