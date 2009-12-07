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

#include "../src/transport/SimpleTransportMaster.hpp"
#include <Tritium/Engine.hpp>
#include <Tritium/Song.hpp>
#include <Tritium/TransportPosition.hpp>
#include <Tritium/Logger.hpp>
#include <Tritium/Preferences.hpp>

#include <stdint.h>  // uint32_t, etc.
#include <cmath>

// CHANGE THIS TO MATCH YOUR FILE:
#define THIS_NAMESPACE t_SimpleTransportMaster
#include "test_macros.hpp"
#include "test_utils.hpp"

using namespace Tritium;

namespace THIS_NAMESPACE
{
    const char song_file_name[] = T_SIMPLETRANSPORTMASTER_H2SONG;

    struct Fixture
    {
	Song* s;
	SimpleTransportMaster x;

	Fixture() {
	    Logger::create_instance();
	    Preferences* prefs = new Preferences();
	    Hydrogen::create_instance(prefs);
	    s = Song::load(song_file_name);
	    x.set_current_song(s);
	}
	~Fixture() {
	    x.set_current_song(0);
	    delete s;
	    delete Hydrogen::get_instance();
	    delete Logger::get_instance();
	}
    };



} // namespace THIS_NAMESPACE

TEST_BEGIN( Fixture );

TEST_CASE( 010_defaults )
{
    TransportPosition pos;

    CK( x.get_state() == TransportPosition::STOPPED );
    CK( 0 == x.get_current_frame() );
    x.get_position(&pos);
    TT_VALID_POS(pos, s);
    CK( pos.bar == 1 );
    CK( pos.beat == 1 );
    CK( pos.tick == 0 );
    CK( pos.bar_start_tick == 0 );
    CK( pos.frame == 0 );
    CK( pos.bbt_offset == 0 );
}

void map_frame_to_bbt(uint32_t frame, uint32_t& bar, uint32_t& beat, uint32_t& tick, uint32_t& bbt_offset,
		      uint32_t& bar_start_tick, double frames_per_tick)
{
    tick = floor( double(frame) / frames_per_tick );
    bbt_offset = frame - round(double(tick) * frames_per_tick);
    bar = (tick / 192) + 1;
    bar_start_tick = (bar - 1) * 192;
    tick = tick % 192;
    beat = (tick / 48) + 1;
    tick = tick % 48;
}


TEST_CASE( 020_start_stop )
{
    TransportPosition pos, posb;
    x.get_position(&pos);
    TT_VALID_POS(pos, s);
    x.processed_frames(1024);
    x.get_position(&posb);
    TT_VALID_POS(posb, s);
    CK( pos.frame == 0 );
    CK( pos.bbt_offset == 0 );
    CK( pos.frame == posb.frame );
    CK( pos.bar == posb.bar );
    CK( pos.beat == posb.beat );
    CK( pos.tick == posb.tick );
    CK( pos.bbt_offset == posb.bbt_offset );

    CK( pos.frame_rate == 48000.0 );
    uint32_t frame, delta = 1237;
    uint32_t tot_frames = pos.frames_per_tick() * 192 * 8;
    uint32_t tpbar = 192;  // ticks per bar
    uint32_t bar, beat, tick, bbt_offset, __bar_start_tick;
    uint32_t fpt, fpb;  // frames per tick, frames per bar
    double tickdrift;

    x.start();
    for( frame = 0 ; frame <= tot_frames ; frame += delta ) {
	x.get_position(&pos);
	TT_VALID_POS(pos, s);

	CK( pos.beats_per_minute == 100.0 );
	map_frame_to_bbt(frame, bar, beat, tick, bbt_offset, __bar_start_tick, pos.frames_per_tick());
	    
	BOOST_MESSAGE("pos B:b.t.o @F = " << pos.bar << ":" << pos.beat << "."
		      << pos.tick << "." << pos.bbt_offset << " @" << pos.frame);
	BOOST_MESSAGE("cal B:b.t.o @F = " << bar << ":" << beat << "."
		      << tick << "." << bbt_offset << " @" << frame);
	CK( pos.frame == frame );
	CK( pos.bar == bar );
	CK( pos.beat == beat );
	tickdrift = fabs(double(pos.tick) - double(tick));
	if( tickdrift > (48.0/2.0) ) {
	    tickdrift -= pos.frames_per_tick();
	}
	CK( fabs(tickdrift) < 1.5 );
	// This drifts to much to check:
	// CK( pos.bbt_offset == bbt_offset );
	CK( pos.bar_start_tick == __bar_start_tick );

	x.processed_frames(delta);
    }

}

TEST_END()
