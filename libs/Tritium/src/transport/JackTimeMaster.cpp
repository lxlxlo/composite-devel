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

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <Tritium/IO/JackOutput.h>
#include "../IO/JackClient.h"
#include "JackTimeMaster.h"

#include <cassert>

using namespace Tritium;

namespace Tritium {
    extern JackOutput* jackDriverInstance;  // Tritium::JackOutput (../IO/jack_output.cpp)
}

bool jack_is_up(void)
{
    bool rv;
    try {
	#warning "This may not be realtime safe: JackClient::get_instance()."
	if( jackDriverInstance
	    && dynamic_cast<JackOutput*>(jackDriverInstance)
	    && JackClient::get_instance()->ref() /* client pointer */ ) {
	    rv = true;
	} else {
	    rv = false;
	}
    } catch (...) {
	rv = false;
    }
    return rv;
}

JackTimeMaster::JackTimeMaster() :
    m_pSong( 0 ),
    m_pBeat( 0 )

{
}

JackTimeMaster::~JackTimeMaster()
{
}

bool JackTimeMaster::setMaster(bool if_none_already)
{
    QMutexLocker mx(&m_mutex);
    if( ! jack_is_up() ) return false;

    int rv = jack_set_timebase_callback( JackClient::get_instance()->ref(),
					 (if_none_already) ? 1 : 0,
					 JackTimeMaster::_callback,
					 (void*)this );
    return (rv == 0);
}

void JackTimeMaster::clearMaster(void)
{
    QMutexLocker mx(&m_mutex);
    if( ! jack_is_up() ) return;
    jack_release_timebase(JackClient::get_instance()->ref()); // ignore return
}

void JackTimeMaster::set_current_song(Song* s)
{
    QMutexLocker mx(&m_mutex);
    m_pSong = s;
}

void JackTimeMaster::_callback(jack_transport_state_t state,
			       jack_nframes_t nframes,
			       jack_position_t* pos,
			       int new_pos,
			       void* arg)
{
    static_cast<JackTimeMaster*>(arg)->callback(state, nframes, pos, new_pos);
}

void JackTimeMaster::callback(jack_transport_state_t state,
			      jack_nframes_t nframes,
			      jack_position_t* pos,
			      int new_pos)
{
    QMutexLocker mx(&m_mutex);

    if(m_pBeat) {
	(*m_pBeat) = true;
    }

    assert(false);
}

