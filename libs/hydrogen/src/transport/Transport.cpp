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

#include <hydrogen/Transport.h>
#include "SimpleTransportMaster.h"

using namespace H2Core;

#warning "Don't do a global object like this."
Transport* Transport::_instance = new Transport();

class H2Core::TransportPrivate
{
public:
    TransportMasterInterface* xport;
};

Transport::Transport() :
    d(0)    
{
    d = new TransportPrivate;
    d->xport = new SimpleTransportMaster;
}

Transport::~Transport()
{
    delete d->xport;
    delete d;
}

Transport* Transport::get_instance(void)
{
    return _instance;
}

int Transport::locate(uint32_t frame)
{
    if(d->xport) return d->xport->locate(frame);
    return -1;
}

int Transport::locate(uint32_t bar, uint32_t beat, uint32_t tick)
{
    if(d->xport) return d->xport->locate(bar, beat, tick);
    return -1;
}

void Transport::start(void)
{
    if(d->xport) d->xport->start();
}

void Transport::stop(void)
{
    if(d->xport) d->xport->stop();
}

void Transport::get_position(TransportPosition* pos)
{
    if(d->xport) d->xport->get_position(pos);
}

void Transport::processed_frames(uint32_t nFrames)
{
    if(d->xport) d->xport->processed_frames(nFrames);
}

void Transport::set_current_song(Song* s)
{
    if(d->xport) d->xport->set_current_song(s);
}
