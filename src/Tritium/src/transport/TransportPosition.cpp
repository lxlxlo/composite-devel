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

#include <Tritium/TransportPosition.hpp>
#include <cstdlib> // rand()
#include <cassert>
#include <cmath>

using namespace Tritium;

/**
 * Transport Position Algebra
 *
 * Suppose the following alignment
 *
 * Pos:                               A
 * Frames: .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .
 * Tick:    5      6      7      8      9      a      b      c  (hex)
 * Beat:   2 (cont.)
 * Bar:    9 (cont.)
 * Tempo:  120 bpm (cont.) (frames per tick = 2.33333)
 *
 * The TransportPosition describes the current /frame/, and its
 * relationship to the Bar:Beat.tick (Bbt) timeline.  You see that
 * there is no frame that is perfectly aligned with tick 8.  That's
 * why there is a bbt_offset parameter that can account for the
 * misalignment between frames and ticks.  In this case, the transport
 * looks like:
 *
 *     A = Bar 9, Beat 2, Tick 8, bbt_offset 1.66666666
 *       = [9:2.8 + 1.66]
 *
 * Now, we wish to do establish some manner of algebra for
 * manipulating the transport position (A) so that we can, for
 * instance, snap to a tick/beat/bar, advance or rewind several ticks,
 * etc.
 *
 * For example, to snap to the nearest tick, I want the frame closes
 * to tick 9.  We can see visually that we advance to the next frame
 * and adjust bbt_offset to .333333.  It's calculated like this:
 *
 *     // SNAPPING CALCULATION
 *     double fpt = A.frames_per_tick();
 *     double df;  // precise distance to destination (real)
 *     double q;   // quantized distance to destination (rounded)
 *     if(bbt_offset > (fpt / 2.0)) {
 *         // Advance
 *         df = fpt - bbt_offset;       // df = .66
 *     } else {
 *         // Reverse
 *         df = - bbt_offset;
 *     }
 *     q = ::round(df);                 // q = 1.0 (quantize)
 *     frame += int(q);                 // Advance 1 frame
 *     bbt_offset = df - q;             // bbt_offset = .33
 *     if( q > .5 ) tick += 1;          // tick = 9
 *
 * Now that we are on a tick, we expect that advancing forward and
 * backwards by one tick will always keep us on the frame closest to
 * the tick.  This means that sometimes the frame changes by 2 and
 * sometimes the frame changes by 3.  Suppose this case, where we wish
 * to reverse by 4 ticks (B to C).
 *
 * Pos:    C                             B
 * Frames: .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .
 * Tick:    5      6      7      8      9      a      b      c  (hex)
 * Beat:   2 (cont.)
 * Bar:    9 (cont.)
 * Tempo:  120 bpm (cont.) (frames per tick = 2.33333)
 *
 *
 *     // MOVING BY WHOLE TICKS, STAYING CLOSE
 *     double fpt = B.frames_per_tick();
 *     double df; // precise distance to destination (real)
 *     double q;  // quantized distance to destination (rounded)
 *     int dt = -4; // tick adjustement;
 *     assert( fabs(bbt_offset) <= .5 ); // already close to tick
 *     df = (dt*fpt) - bbt_offset;           // df = -9.6666
 *     q = ::round(df);                      // q = -10
 *     frame += int(q);
 *     bbt_offset = q - df;                  // bbt_offset = -.33
 *     tick += dt;                           // tick = 5
 *
 * Notice that bbt_offset is now negative (-.33).  What's the
 * alternative? fpt - .33 = 2 and tick = 4.  If you're sequencing,
 * this is a real pain when the frame you are it is tick 4.9999999.
 * When we snap to a tick, we want the frame to be very /close/ and
 * the value of the tick to change in a regular manner (3, 4, 5, 6,
 * ...).  The WHOLE POINT of TransportPosition is to make sequencing
 * easy.  Therefore, the normalized range for bbt_offset is [-.5,
 * fpt-.5).
 *
 * But what if I'm not already snapped to a tick (fabs(bbt_offset) >
 * .5)... and I wish to advance/reverse by a tick?  What does that
 * mean?
 *
 * Or, stated another way, what if I have meticulously calculated a
 * bbt_offset? When I increment and decrement by one tick, I hope to
 * keep that offset in-tact as much as possible?  (E.g. if a sequencer
 * is going 80 bpm with 48 ticks per beat... that's 16 ms per tick of
 * resolution -- which isn't always good enough.)  For example,
 * suppose I want to move like this (from D to E to F):
 *
 * Pos (ideal):               D      E      F
 * Pos (quant):              D        E     F
 * Frames: .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .
 * Tick:    5      6      7      8      9      a      b      c  (hex)
 * Beat:   2 (cont.)
 * Bar:    9 (cont.)
 * Tempo:  120 bpm (cont.) (frames per tick = 2.33333)
 *
 * (i.e. I want to maintain a 1.33 bbt_offset as close as possible
 * when moving across ticks.)  Obviously, doing this will require that
 * I maintain bbt_offset_desired in addition to the actual bbt_offset.
 * If you want this degree of resolution, it is better to handle this
 * yourself rather than bog down the rest of the application.
 *
 * Therefore, here is what we'll do:
 *
 *     + If fabs(bbt_offset) <= .5 we will always choose
 *       the frame closest to the tick.  There will be
 *       no rounding accumulation error (frame drift)
 *       in this case.
 *
 *     + Otherwise, we will adjust the frame by
 *       ::round(frames_per_tick() + dither()), and
 *       there will be a small accumulation error
 *       (frame drift).
 *
 */


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
 * To simplify calculations, many internal methods will adjust the
 * frame, bbt_offset, tick, bar, beat, etc... so that they don't
 * actually make sense.  E.g. bar 3 beat -1.  This function will set
 * these right and into their normal ranges (e.g. bar 2 beat 3).
 *
 * This function does _not_ adjust the frame, only the Bbt data
 * describing the frame.
 */
void TransportPosition::normalize()
{
    double fpt = frames_per_tick();

    /* bbt_offset should be in the range [-.5, fpt-.5)
     */
    if( (bbt_offset < -.5) || (bbt_offset >= fpt-.5) ) {
	double dt = ::floor(bbt_offset / fpt);
	bbt_offset -= dt * fpt;
	tick += dt;
	assert( fabs(bbt_offset) <= fpt );
	if( bbt_offset < -.5 ) {
	    bbt_offset += fpt;
	    --tick;
	}
    }
    assert( bbt_offset >= -.5 );
    assert( bbt_offset < fpt - .5 );

    /* tick should be in range [0, ticks_per_beat)
     */
    while(tick < 0) {
	--beat;
	tick += ticks_per_beat;
    }
    while((tick > 0) && (unsigned(tick) >= ticks_per_beat)) {
	++beat;
	tick -= ticks_per_beat;
    }

    /* beat should be in range [1, beats_per_bar]
     */
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

    /* bar should be in range [1, +inf)
     */
    if( bar < 1 ) {
	bar = 1;
	beat = 1;
	tick = 0;
	bbt_offset = 0;
	bar_start_tick = 0;
	frame = 0;
    }    
}

// XXX TODO
// Maybe change this to snap_to_frame() or locate() ?
void TransportPosition::normalize(uint32_t snap_to_frame)
{
    normalize();
    if( (snap_to_frame < frame) && ((frame - snap_to_frame) > bbt_offset) ) {
	--(*this);
    }
    if( snap_to_frame == frame ) return;

    if( snap_to_frame > frame ) {
	bbt_offset += snap_to_frame - frame;
	frame = snap_to_frame;
    } else {
	uint32_t diff;
	diff = frame - snap_to_frame;
	assert( diff <= bbt_offset );
	bbt_offset -= diff;
	frame = snap_to_frame;
    }

    assert( bbt_offset >= -0.5 );
    assert( bbt_offset < (frames_per_tick()-.5) );
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
    double d_tick = double(tick) + bbt_offset/double(frames_per_tick());
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
	if( bbt_offset >= ( frames_per_tick() / 2.0 ) ) {
	    ceil(TICK);
	} else {
	    floor(TICK);
	}
	break;
    }
}

void TransportPosition::ceil(TransportPosition::snap_type s)
{
    double df, q;
    double fpt = frames_per_tick();
    normalize();
    switch(s) {
    case BAR:
	if((beat == 1)
	   && (tick == 0)
	   && (fabs(bbt_offset) <= 0.5)
	    ) break;
	df = (beats_per_bar * ticks_per_beat) * fpt; // Frames in full measure.
	df -= fpt * ((beat-1) * ticks_per_beat + tick) + bbt_offset;
	q = ::round(df);
	assert(q > 0.0);
	frame += unsigned(q);
	++bar;
	beat = 1;
	tick = 0;
	bbt_offset = q - df;
	bar_start_tick += beats_per_bar * ticks_per_beat;
	break;
    case BEAT:
	if((tick == 0)
	   && (fabs(bbt_offset) <= 0.5)
	    ) break;
	df = ticks_per_beat * fpt; // Frames in full beat
	df -= tick * fpt + bbt_offset;
	q = ::round(df);
	assert(q > 0.0);
	frame += unsigned(q);
	++beat;
	tick = 0;
	bbt_offset = q - df;
	normalize();
	break;
    case TICK:
	if(fabs(bbt_offset) < .5) break;
	df = (fpt - bbt_offset);
	q = ::round(df);
	assert(q > 0.0);
	frame += unsigned(q);
	++tick;
	bbt_offset = q - df;
	normalize();
	break;
    }
    assert(bbt_offset >= -0.5);
    assert(bbt_offset < fpt - .5);
}

void TransportPosition::floor(TransportPosition::snap_type s)
{
    double df, q;
    double fpt = frames_per_tick();
    double ticks;

    normalize();  // Code is assuming that we are normalized.
    switch(s) {
    case BAR:
	if( (beat == 1)
	    && (tick == 0)
	    && (fabs(bbt_offset) <= .5)
	    ) break;
	df = fpt * ((beat - 1) * ticks_per_beat + tick) + bbt_offset;
	q = ::round(df);
	if( frame > q ) {
	    frame -= q;
	    bbt_offset = df - q;
	} else {
	    frame = 0;
	    bbt_offset = 0;
	}
	beat = 1;
	tick = 0;
	ticks = beats_per_bar * ticks_per_beat;
	if( bar_start_tick > ticks ) {
	    bar_start_tick -= ticks;
	} else {
	    bar_start_tick = 0;
	}
	break;
    case BEAT:
	if( (tick == 0)
	    && (fabs(bbt_offset) <= .5)
	    ) break;
	df = tick * fpt + bbt_offset;
	q = ::round(df);
	if( frame > q ) {
	    frame -= q;
	    bbt_offset = df - q;
	} else {
	    frame = 0;
	    bbt_offset = 0;
	}
	tick = 0;
	break;
    case TICK:
	if( fabs(bbt_offset) <= .5 ) break;
	df = bbt_offset;
	q = ::round(df);
	if( frame > q ) {
	    frame -= q;
	    bbt_offset = df - q;
	} else {
	    frame = 0;
	    bbt_offset = 0;
	}
	break;
    }
    assert(bbt_offset >= -0.5);
    assert(bbt_offset < fpt - .5);
}

TransportPosition& TransportPosition::operator++()
{
    return operator+=(1);
}

TransportPosition& TransportPosition::operator--()
{
    return operator+=(-1);
}

TransportPosition Tritium::operator+(const TransportPosition& pos, int ticks)
{
    TransportPosition rv(pos);
    rv += ticks;
    return rv;
}

TransportPosition Tritium::operator-(const TransportPosition& pos, int ticks)
{
    return operator+(pos, -ticks);
}

TransportPosition& TransportPosition::operator+=(int ticks)
{
    if( ticks == 0 ) return *this;
    double fpt = frames_per_tick();
    double df; // precise distance to destination (real)
    double q;  // quantized distance to destination (rounded)
    bool snapped = false; // whether or not to snap to ticks
    if( fabs(bbt_offset) <= .5 ) {
	snapped = true;
    }
    if( snapped ) {
	df = (ticks*fpt) - bbt_offset;
    } else {
	df = ticks*fpt + dither();
    }
    q = ::round(df);
    if( frame >= (-q) ) {
	frame += int(q);
	if( snapped ) {
	    bbt_offset = q - df;
	} else {
	    // .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .
	    //  5      6      7      8      9      a      b      c
	    //                         A=== +3 ============>|
	    //                         bbto = bbto + q - (ticks*fpt)
	    //                              = .66 + 7 - (3*2.333)
	    //                              = .66
	    // .5 .  . 6.  .  7  .  .8 .  . 9.  .  a  .  .b .  . c.
	    //    |<===== -3 ==========A
	    //      bbto = bbto + q - (ticks * fpt)
	    //           = .66 - 7 - (-3 * 2.3333)
	    //           = .66
	    // .5 .  . 6.  .  7  .  .8 .  . 9.  .  a  .  .b .  . c.
	    // |<=============== -4 =========A
	    //      bbto = bbto + q - (ticks * fpt)
	    //           = .33 + (-10) - (-4 * 2.3333)
	    //           = -.33
	    bbt_offset = bbt_offset + q - (ticks*fpt);
	}
	tick += ticks;
    } else {
	frame = 0;
	bbt_offset = 0;
	tick += ticks;
    }
    normalize();
    return *this;
}

TransportPosition& TransportPosition::operator-=(int ticks)
{
    return this->operator+=(-ticks);
}

