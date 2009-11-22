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
#ifndef H2CORE_JACKTIMEMASTER_H
#define H2CORE_JACKTIMEMASTER_H

#include <QtCore/QMutex>
#include <jack/transport.h>
#include <Tritium/JackTimeMasterEvents.h>

namespace Tritium
{
    class Song;

    class JackTimeMaster
    {
    public:
	JackTimeMaster();
	~JackTimeMaster();

	bool setMaster(bool if_none_already = false);
	void clearMaster(void);
	void set_current_song(Song* s);
	void set_heartbeat(bool* beat);

    private:
	static void _callback(jack_transport_state_t state,
			      jack_nframes_t nframes,
			      jack_position_t* pos,
			      int new_pos,
			      void* arg);
	void callback(jack_transport_state_t state,
		      jack_nframes_t nframes,
		      jack_position_t* pos,
		      int new_pos);

    private:
	Song* m_pSong;
	bool* m_pBeat;
	QMutex m_mutex;
    }; // class JackTimeMaster

/////////////////////////////////////////////////////////////
// FOR THE EVENT QUEUE DEFINES, SEE JackTimeMasterEvents.h //
/////////////////////////////////////////////////////////////

} // namespace Tritium

#endif // H2CORE_H2TRANSPORT_H
