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

#include "H2Transport.h"
#include "SimpleTransportMaster.h"

using namespace H2Core;

class H2Core::H2TransportPrivate
{
public:
    Transport* xport;
};

H2Transport::H2Transport() :
    d(0)    
{
    d = new H2TransportPrivate;
    d->xport = new SimpleTransportMaster;
}

H2Transport::~H2Transport()
{
    delete d->xport;
    delete d;
}

int H2Transport::locate(uint32_t frame)
{
    if(d->xport) return d->xport->locate(frame);
    return -1;
}

int H2Transport::locate(uint32_t bar, uint32_t beat, uint32_t tick)
{
    if(d->xport) return d->xport->locate(bar, beat, tick);
    return -1;
}

void H2Transport::start(void)
{
    if(d->xport) d->xport->start();
}

void H2Transport::stop(void)
{
    if(d->xport) d->xport->stop();
}

void H2Transport::get_position(TransportPosition* pos)
{
    if(d->xport) d->xport->get_position(pos);
}

void H2Transport::processed_frames(uint32_t nFrames)
{
    if(d->xport) d->xport->processed_frames(nFrames);
}

void H2Transport::set_current_song(Song* s)
{
    if(d->xport) d->xport->set_current_song(s);
}
