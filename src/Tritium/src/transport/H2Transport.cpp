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

#include <memory>
#include <Tritium/event_queue.h>
#include "H2Transport.h"
#include "SimpleTransportMaster.h"
#include "JackTimeMaster.h"

using namespace Tritium;

class Tritium::H2TransportPrivate
{
public:
    std::auto_ptr<Transport> xport;

    /* This is used as a heartbeat signal with the JACK transport.
     * It always sets it true.  We always set it false.  If it's
     * ever false, then we're no longer being called... and probably
     * no longer the transport master.
     */
    bool presumed_jtm;  // We *think* we're the Jack time master.
    bool heartbeat_jtm;
    std::auto_ptr<JackTimeMaster> jtm;
    Song* pSong;  // Cached pointer for JTM
};

H2Transport::H2Transport() :
    d(0)    
{
    d = new H2TransportPrivate;
    d->xport.reset( new SimpleTransportMaster );
    d->presumed_jtm = false;
    d->heartbeat_jtm = false;
    d->pSong = 0;
}

H2Transport::~H2Transport()
{
    delete d;
}

int H2Transport::locate(uint32_t frame)
{
    if(d->xport.get()) return d->xport->locate(frame);
    return -1;
}

int H2Transport::locate(uint32_t bar, uint32_t beat, uint32_t tick)
{
    if(d->xport.get()) return d->xport->locate(bar, beat, tick);
    return -1;
}

void H2Transport::start(void)
{
    EventQueue::get_instance()->push_event( EVENT_TRANSPORT, (int)TransportPosition::ROLLING );
    if(d->xport.get()) d->xport->start();
}

void H2Transport::stop(void)
{
    EventQueue::get_instance()->push_event( EVENT_TRANSPORT, (int)TransportPosition::STOPPED );
    if(d->xport.get()) d->xport->stop();
}

void H2Transport::get_position(TransportPosition* pos)
{
    if(d->xport.get()) d->xport->get_position(pos);
}

void H2Transport::processed_frames(uint32_t nFrames)
{
    if( d->heartbeat_jtm == false && d->presumed_jtm == true ) {
	EventQueue::get_instance()->push_event( EVENT_JACK_TIME_MASTER, JACK_TIME_MASTER_NO_MORE );
	d->presumed_jtm = false;
    }
    d->heartbeat_jtm = false;

    if(d->xport.get()) d->xport->processed_frames(nFrames);
}

void H2Transport::set_current_song(Song* s)
{
    d->pSong = s;
    if( d->jtm.get() ) {
	d->jtm->set_current_song(s);
    }
    if(d->xport.get()) d->xport->set_current_song(s);
}

uint32_t H2Transport::get_current_frame()
{
    if(d->xport.get()) {
	return d->xport->get_current_frame();
    }
    return (uint32_t)-1;
}

TransportPosition::State H2Transport::get_state()
{
    if(d->xport.get()) {
	return d->xport->get_state();
    }
    return TransportPosition::STOPPED;
}

bool H2Transport::setJackTimeMaster(bool if_none_already)
{
    bool rv;

    if( ! d->jtm.get() ) {
	d->jtm.reset( new JackTimeMaster );
	d->jtm->set_current_song( d->pSong );
    }

    rv = d->jtm->setMaster(if_none_already);
    if( rv ) {
	EventQueue::get_instance()->push_event( EVENT_JACK_TIME_MASTER, JACK_TIME_MASTER_NOW );
    }
    return rv;
}

void H2Transport::clearJackTimeMaster()
{
    if( d->jtm.get() ) {
	d->jtm->clearMaster();
	EventQueue::get_instance()->push_event( EVENT_JACK_TIME_MASTER, JACK_TIME_MASTER_NO_MORE );
    }
}

bool H2Transport::getJackTimeMaster()
{
    return d->presumed_jtm;
}
