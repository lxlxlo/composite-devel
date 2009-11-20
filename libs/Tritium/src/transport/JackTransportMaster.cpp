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

#include <Tritium/Transport.h>
#include <Tritium/TransportPosition.h>
#include "JackTransportMaster.h"

#include <jack/jack.h>
#include <jack/transport.h>

#include <cmath>

#warning "JackTransportMaster is **NOT** implemented yet."
#warning "We have not handled the client pointer or set up any callbacks yet."

using namespace H2Core;

class H2Core::JackTransportMasterPrivate
{
public:
    jack_client_t* client;
    uint32_t next_frame;
};

JackTransportMaster::JackTransportMaster(void) : d(0)
{
    d = new JackTransportMasterPrivate;
    d->client = 0;
    d->next_frame = (uint32_t)-1;
}

int JackTransportMaster::locate(uint32_t frame)
{
    return jack_transport_locate(d->client, frame);
}

int JackTransportMaster::locate(uint32_t bar, uint32_t beat, uint32_t tick)
{
    jack_position_t pos;
    pos.valid = (jack_position_bits_t)(JackPositionBBT | JackBBTFrameOffset);
    pos.bar = bar;
    pos.beat = beat;
    pos.tick = tick;
    pos.bbt_offset = 0;
    #warning "Did we fill out enough stuff?"
    return jack_transport_reposition(d->client, &pos);
}

void JackTransportMaster::start(void)
{
    jack_transport_start(d->client);
}

void JackTransportMaster::stop(void)
{
    jack_transport_stop(d->client);
}

void JackTransportMaster::get_position(TransportPosition* hpos)
{
    jack_position_t jpos;
    jack_transport_state_t state;

    if( hpos == 0 ) return;
    state = jack_transport_query(d->client, &jpos);

    if( state == JackTransportRolling ) {
        hpos->state = TransportPosition::ROLLING;
    } else {
        hpos->state = TransportPosition::STOPPED;
    }
    hpos->new_position = ( jpos.frame != d->next_frame );
    hpos->frame = jpos.frame;
    hpos->frame_rate = jpos.frame_rate;
    #warning "Did not check for a jpos.valid & JackPositionBBT"
    hpos->bar = jpos.bar;
    hpos->beat = jpos.beat;
    hpos->tick = jpos.tick;
    hpos->bbt_offset = (jpos.valid & JackBBTFrameOffset) ? jpos.bbt_offset : 0;
    hpos->bar_start_tick = round(jpos.bar_start_tick);
    hpos->beats_per_bar = floor(jpos.beats_per_bar);
    hpos->beat_type = floor(jpos.beat_type);
    hpos->ticks_per_beat = round(jpos.ticks_per_beat);
    hpos->beats_per_minute = jpos.beats_per_minute;
}

void JackTransportMaster::processed_frames(uint32_t nFrames)
{
    jack_transport_state_t state;
    jack_position_t jpos;

    state = jack_transport_query(d->client, &jpos);
    if( state == JackTransportRolling ) {
        d->next_frame = jpos.frame + nFrames;
    } else {
        d->next_frame = jpos.frame;
    }
}

void JackTransportMaster::set_current_song(Song* s)
{
    // Timeline handled by JACK server.
}

uint32_t JackTransportMaster::get_current_frame(void)
{
    return jack_get_current_transport_frame(d->client);
}
