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

/**
 * This should probably be a member function... but for
 * now it's here to avoid recompiling all of hydrogen.
 */
void TransportPosition::normalize()
{
    double fpt = frames_per_tick();
    /* bbt_offset is unsigned.  If it's ever signed, then....
    while(bbt_offset < 0) {
	--tick;
	bbt_offset += ::round(fpt + dither());
    }
    */
    if( bbt_offset > fpt ) {
	double n, f;
	f = fpt * modf(double(bbt_offset)/fpt, &n);
	f += dither();
	tick += int(n);
	frame += bbt_offset - f;	
	bbt_offset = f;
    }
    while(tick < 0) {
	--beat;
	tick += ticks_per_beat;
    }
    while((tick > 0) && (unsigned(tick) >= ticks_per_beat)) {
	++beat;
	tick -= ticks_per_beat;
    }
    while(beat < 1) {
	uint32_t ticks;
	--bar;
	ticks = beats_per_bar * ticks_per_beat;
	if( bar_start_tick > ticks ) {
	    bar_start_tick -= ticks;
	} else {
	    bar_start_tick = 0;
	}
	beat += beats_per_bar;
    }
    while(beat > beats_per_bar) {
	++bar;
	bar_start_tick += beats_per_bar * ticks_per_beat;
	beat -= beats_per_bar;
    }
    if( bar < 1 ) {
	bar = 1;
	beat = 1;
	tick = 0;
	bbt_offset = 0;
	bar_start_tick = 0;
	frame = 0;
    }    
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
    double d_beat = double(beat - 1) + d_tick / double(ticks_per_beat);
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

void TransportPosition::ceil(TransportPosition::snap_type s)
{
    double df;
    double fpt = frames_per_tick();
    normalize();
    switch(s) {
    case BAR:
	if((beat == 1) && (tick == 0) && (bbt_offset == 0)) break;
	df = (beats_per_bar * ticks_per_beat) * fpt; // Frames in full measure.
	df -= ((beat-1) * ticks_per_beat
	       + tick) * fpt
	    + bbt_offset;
	frame += ::round(df + dither());
	++bar;
	beat = 1;
	tick = 0;
	bbt_offset = 0;
	bar_start_tick += beats_per_bar * ticks_per_beat;
	break;
    case BEAT:
	if((tick == 0) && (bbt_offset == 0)) break;
	df = ticks_per_beat * fpt; // Frames in full beat
	df -= tick * fpt + bbt_offset;
	frame += ::round(df + dither());
	++beat;
	tick = 0;
	bbt_offset = 0;
	normalize();
	break;
    case TICK:
	if(bbt_offset == 0) break;
	df = (fpt - bbt_offset);
	frame += ::round(df + dither());
	++tick;
	bbt_offset = 0;
	normalize();
	break;
    }
}

void TransportPosition::floor(TransportPosition::snap_type s)
{
    double df;
    double fpt = frames_per_tick();
    double ticks;

    normalize();  // Code is assuming that we are normalized.
    switch(s) {
    case BAR:
	df = ((beat - 1) * ticks_per_beat
	      + tick) * fpt
	    + bbt_offset;
	df = ::round(df + dither());
	if( frame > df ) {
	    frame -= df;
	} else {
	    frame = 0;
	}
	beat = 1;
	tick = 0;
	bbt_offset = 0;
	ticks = beats_per_bar * ticks_per_beat;
	if( bar_start_tick > ticks ) {
	    bar_start_tick -= ticks;
	} else {
	    bar_start_tick = 0;
	}
	break;
    case BEAT:
	df = tick * fpt
	    + bbt_offset;
	df = ::round(df + dither());
	if( frame > df ) {
	    frame -= df;
	} else {
	    frame = 0;
	}
	tick = 0;
	bbt_offset = 0;
	break;
    case TICK:
	if( frame > bbt_offset ) {
	    frame -= bbt_offset;
	} else {
	    frame = 0;
	}
	bbt_offset = 0;
	break;
    }
}

TransportPosition& TransportPosition::operator++()
{
    ++tick;
    frame += ::round(frames_per_tick() + dither());
    normalize();
    return *this;
}

TransportPosition& TransportPosition::operator--()
{
    --tick;
    uint32_t fpt = ::round(frames_per_tick() + dither());
    if( frame > fpt ) {
	frame -= fpt;
    } else {
	frame = 0;
    }
    normalize();
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
    double df = ticks * frames_per_tick();
    tick += ticks;
    if( (df < 0.0) && (-df > frame) ) {
	frame = 0;
    } else {
	frame += df;
    }
    normalize();
    return *this;
}

TransportPosition& TransportPosition::operator-=(int ticks)
{
    return this->operator+=(-ticks);
}

