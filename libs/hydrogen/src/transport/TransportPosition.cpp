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
#include <cstdlib> // rand()
#include <cmath>

using namespace H2Core;

/**
 * Returns a random number in the range [-0.5, 0.5]
 *
 * This is used primarily for frame adjustment calculations to
 * mitigate drifting from roundoff errors without actually
 * keeping track of a fractional frame value.
 */
inline double dither()
{
    return double(rand())/double(RAND_MAX) - 0.5;
}

TransportPosition::TransportPosition() :
    state( STOPPED ),
    new_position( true ),
    frame( 0 ),
    frame_rate( 48000 ),
    bar( 1 ),
    beat( 1 ),
    tick( 0 ),
    bbt_offset( 0 ),
    bar_start_tick( 0 ),
    beats_per_bar( 4 ),
    beat_type( 4 ),
    ticks_per_beat( 48 ),
    beats_per_minute( 120.0 )
{
}

TransportPosition::TransportPosition(const TransportPosition& o) :
    state( o.state ),
    new_position( o.new_position ),
    frame( o.frame ),
    frame_rate( o.frame_rate ),
    bar( o.bar ),
    beat( o.beat ),
    tick( o.tick ),
    bbt_offset( o.bbt_offset ),
    bar_start_tick( o.bar_start_tick ),
    beats_per_bar( o.beats_per_bar ),
    beat_type( o.beat_type ),
    ticks_per_beat( o.ticks_per_beat ),
    beats_per_minute( o.beats_per_minute )
{
}

void TransportPosition::round(TransportPosition::snap_type s)
{
    double d_tick = double(tick) + double(bbt_offset)/double(frames_per_tick());
    double d_beat = double(beat) + d_tick / double(ticks_per_beat);
    switch(s) {
    case BAR:
	if( d_beat >= double(beats_per_bar)/2.0) {
	    ceil(BAR);
	} else {
	    floor(BAR);
	}
	break;
    case BEAT:
	if( d_tick >= double(ticks_per_beat) / 2.0 ) {
	    ceil(BEAT);
	} else {
	    floor(BEAT);
	}
    case TICK:
	if( double(bbt_offset) >= ( frames_per_tick() / 2.0 ) ) {
	    ceil(TICK);
	} else {
	    floor(TICK);
	}
	break;
    }
}

#warning "Need to implement TransportPosition::ceil() and ::floor()"
void TransportPosition::ceil(TransportPosition::snap_type s)
{
    switch(s) {
    case BAR:
	ceil(BEAT);
	frame += (frames_per_tick() * ticks_per_beat * (beats_per_bar - beat) );
	++bar;
	beat = 1;
	break;
    case BEAT:
	ceil(TICK);
	frame += (frames_per_tick() * (ticks_per_beat-tick));
	++beat;
	tick = 0;
	break;
    case TICK:
	frame += (frames_per_tick() - bbt_offset);
	++tick;
	bbt_offset = 0;
	break;
    }
    while( tick >= ticks_per_beat ) {
	++beat;
	tick -= ticks_per_beat;
    }
    while( beat > beats_per_bar ) {
	++bar;
	beat -= beats_per_bar;
    }
}

void TransportPosition::floor(TransportPosition::snap_type s)
{
    uint32_t tmp;
    switch(s) {
    case BAR:
	floor(BEAT);
	tmp = frames_per_tick() * ticks_per_beat * (beat-1);
	if( tmp > frame ) {
	    frame -= tmp;
	} else {
	    frame = 0;
	}
	beat = 1;
	break;
    case BEAT:
	floor(TICK);
	tmp = frames_per_tick() * tick;
	if( tmp > frame ) {
	    frame -= tmp;
	} else {
	    frame = 0;
	}
	tick = 0;
	break;
    case TICK:
	if( bbt_offset > frame ) {
	    frame -= bbt_offset;
	} else {
	    frame = 0;
	}
	bbt_offset = 0;
	break;
    }
    while( tick < 0 ) {
	--beat;
	tick += ticks_per_beat;
    }
    while( beat < 1 ) {
	--bar;
	beat += beats_per_bar;
    }
}

TransportPosition& TransportPosition::operator++()
{
    ++tick;
    frame += ::round(frames_per_tick() + dither());
    if( tick >= ticks_per_beat ) {
	beat += tick / ticks_per_beat;
	tick %= ticks_per_beat;
	if( beat > beats_per_bar ) {
	    bar += ((beat-1) / beats_per_bar);
	    beat = 1 + ((beat-1) % beats_per_bar);
	}
    }
    return *this;
}

TransportPosition& TransportPosition::operator--()
{
    if( tick > 0 ) {
	--tick;
    } else {
	tick = ticks_per_beat - 1;
	if( beat > 1 ) {
	    --beat;
	} else {
	    beat = beats_per_bar;
	    if( bar > 1 ) {
		--bar;
	    } else {
		bar = 1;
	    }
	}
    }
    uint32_t fpt = ::round(frames_per_tick() + dither());
    if( frame > fpt ) {
	frame -= fpt;
    } else {
	frame = 0;
    }
    return *this;
}

TransportPosition H2Core::operator+(const TransportPosition& pos, int ticks)
{
    TransportPosition rv(pos);
    rv += ticks;
    return rv;
}

TransportPosition H2Core::operator-(const TransportPosition& pos, int ticks)
{
    return operator+(pos, -ticks);
}

TransportPosition& TransportPosition::operator+=(int ticks)
{
    tick += ticks;
    while( tick < 0 ) {
	--beat;
	tick += ticks_per_beat;
    }
    while( beat < 1 ) {
	--bar;
	beat += beats_per_bar;
    }
    while( tick >= ticks_per_beat ) {
	++beat;
	tick -= ticks_per_beat;
    }
    while( beat > beats_per_bar ) {
	++bar;
	beat -= beats_per_bar;
    }
    return *this;
}

TransportPosition& TransportPosition::operator-=(int ticks)
{
    return this->operator+=(-ticks);
}

