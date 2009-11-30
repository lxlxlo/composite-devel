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

#include <Tritium/Transport.hpp>
#include <Tritium/TransportPosition.hpp>
#include "SimpleTransportMaster.hpp"

#include <Tritium/Song.hpp>

#include <jack/transport.h>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include "songhelpers.hpp"
#include <cmath>
#include <cassert>

using namespace Tritium;

class Tritium::SimpleTransportMasterPrivate
{
public:
    SimpleTransportMasterPrivate();

    void set_current_song(Song* song);

    TransportPosition pos;
    QMutex pos_mutex;
    Song* song;
};

SimpleTransportMasterPrivate::SimpleTransportMasterPrivate() : song(0)
{
    set_current_song(song);
}

SimpleTransportMaster::SimpleTransportMaster(void) : d(0)
{
    d = new SimpleTransportMasterPrivate;
}

int SimpleTransportMaster::locate(uint32_t frame)
{
    QMutexLocker lk(&d->pos_mutex);

    d->pos.ticks_per_beat = d->song->get_resolution();
    d->pos.beats_per_minute = d->song->get_bpm();
    double frames_per_tick =
        double(d->pos.frame_rate)
        * 60.0
        / d->pos.beats_per_minute
        / double(d->pos.ticks_per_beat);
    uint32_t abs_tick = round( double(frame) / frames_per_tick );

    d->pos.bbt_offset = round(fmod(frame, frames_per_tick));
    d->pos.bar = bar_for_absolute_tick(d->song, abs_tick);
    d->pos.bar_start_tick = bar_start_tick(d->song, d->pos.bar);
    d->pos.beat = 1 + (abs_tick - d->pos.bar_start_tick) / d->pos.ticks_per_beat;
    d->pos.tick = (abs_tick - d->pos.bar_start_tick) % d->pos.ticks_per_beat;
    d->pos.frame = frame;
    d->pos.new_position = true;
    return 0;
}

int SimpleTransportMaster::locate(uint32_t bar, uint32_t beat, uint32_t tick)
{
    QMutexLocker lk(&d->pos_mutex);

    d->pos.ticks_per_beat = d->song->get_resolution();
    d->pos.beats_per_minute = d->song->get_bpm();

    #warning "There needs to be input checking here."
    d->pos.bar = bar;
    d->pos.beat = beat;
    d->pos.tick = tick;
    d->pos.bbt_offset = 0;
    uint32_t abs_tick = 0;
    uint32_t t;
    if( bar > song_bar_count(d->song) ) {
        d->pos.beats_per_bar = 4;
        abs_tick = song_tick_count(d->song)
            + (bar - song_bar_count(d->song)) * d->pos.beats_per_bar * d->pos.ticks_per_beat
            + (beat - 1) * d->pos.ticks_per_beat
            + tick;
    } else {
	t = ticks_in_bar(d->song, bar);
        d->pos.beats_per_bar = t / d->pos.ticks_per_beat;
	assert( (t % d->pos.ticks_per_beat) == 0 );
        abs_tick = bar_start_tick(d->song, bar)
            + (beat - 1) * d->pos.ticks_per_beat
            + tick;
    }

    d->pos.frame =
        double(abs_tick)
        * d->pos.frame_rate
        * 60.0
        / double(d->pos.ticks_per_beat)
        / d->pos.beats_per_minute;

    d->pos.new_position = true;

    return 0;
}

void SimpleTransportMaster::start(void)
{
    d->pos.state = TransportPosition::ROLLING;
}

void SimpleTransportMaster::stop(void)
{
    d->pos.state = TransportPosition::STOPPED;
}

void SimpleTransportMaster::get_position(TransportPosition* hpos)
{
    QMutexLocker lk(&d->pos_mutex);
    hpos->state = d->pos.state;
    hpos->frame = d->pos.frame;
    hpos->frame_rate = d->pos.frame_rate;
    hpos->bar = d->pos.bar;
    hpos->beat = d->pos.beat;
    hpos->tick = d->pos.tick;
    hpos->bbt_offset = d->pos.bbt_offset;
    hpos->bar_start_tick = d->pos.bar_start_tick;
    hpos->beats_per_bar = d->pos.beats_per_bar;
    hpos->beat_type = d->pos.beat_type;
    hpos->ticks_per_beat = d->pos.ticks_per_beat;
    hpos->beats_per_minute = d->pos.beats_per_minute;
}

void SimpleTransportMaster::processed_frames(uint32_t nFrames)
{
    QMutexLocker lk(&d->pos_mutex);

    if( d->pos.state == TransportPosition::STOPPED ) {
	return;
    }

    uint32_t target = d->pos.frame + nFrames;
    uint32_t old_bar = d->pos.bar;
    d->pos.frame += nFrames;
    d->pos.bbt_offset += nFrames;
    d->pos.new_position = false;
    d->pos.normalize(target);

    if( old_bar != d->pos.bar ) {
	uint32_t song_bars = song_bar_count(d->song);
	if( d->pos.bar > song_bars ) {
	    d->pos.bar = 1 + ((d->pos.bar - 1) % song_bars);
	    d->pos.bar_start_tick = bar_start_tick(d->song, d->pos.bar); 
	}
        d->pos.beats_per_bar = ticks_in_bar(d->song, d->pos.bar)
            / d->pos.ticks_per_beat;
    }
    // After all the calculations... *now* the new tempo
    // takes effect (for the next cycle).
    d->pos.beats_per_minute = d->song->get_bpm();
}

void SimpleTransportMaster::set_current_song(Song* s)
{
    d->set_current_song(s);
}

uint32_t SimpleTransportMaster::get_current_frame(void)
{
    return d->pos.frame;
}

void SimpleTransportMasterPrivate::set_current_song(Song* s)
{
    QMutexLocker lk(&pos_mutex);
    song = s;

    #warning "Still have a hard-coded frame rate"
    if( song != 0 ) {
        pos.state = TransportPosition::STOPPED;
        pos.frame = 0;
        pos.frame_rate = 48000;
        pos.bar = 1;
        pos.beat = 1;
        pos.tick = 0;
        pos.bbt_offset = 0;
        pos.bar_start_tick = 0;
        pos.beats_per_bar = double(ticks_in_bar(s, 1)) / 48.0;
        pos.beat_type = 4; // Assumed.
        pos.ticks_per_beat = song->get_resolution();
        pos.beats_per_minute = song->get_bpm();
    } else {
        pos.state = TransportPosition::STOPPED;
        pos.frame = 0;
        pos.frame_rate = 48000;
        pos.bar = 1;
        pos.beat = 1;
        pos.tick = 0;
        pos.bbt_offset = 0;
        pos.bar_start_tick = 0;
        pos.beats_per_bar = 4;
        pos.beat_type = 4;
        pos.ticks_per_beat = 48.0;
        pos.beats_per_minute = 120.0;
    }
}

TransportPosition::State SimpleTransportMaster::get_state()
{
    return d->pos.state;
}
