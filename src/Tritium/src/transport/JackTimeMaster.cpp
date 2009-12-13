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

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <Tritium/IO/AudioOutput.hpp>
#include <Tritium/IO/JackOutput.hpp>
#include <Tritium/Engine.hpp>
#include "../IO/JackClient.hpp"
#include "JackTimeMaster.hpp"

#include <cassert>

using namespace Tritium;

JackTimeMaster::JackTimeMaster(JackClient* parent) :
    m_jack_client(parent),
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
    if( ! m_jack_client->jack_is_up() ) return false;

    int rv = jack_set_timebase_callback( m_jack_client->ref(),
					 (if_none_already) ? 1 : 0,
					 JackTimeMaster::_callback,
					 (void*)this );
    return (rv == 0);
}

void JackTimeMaster::clearMaster(void)
{
    QMutexLocker mx(&m_mutex);
    if( ! m_jack_client->jack_is_up() ) return;
    jack_release_timebase(m_jack_client->ref()); // ignore return
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

